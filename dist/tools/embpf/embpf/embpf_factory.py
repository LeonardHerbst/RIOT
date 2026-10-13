import logging
import struct

from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import Relocation
from typing import Dict, Type, IO
from abc import ABC

from embpf.embpf import EmBPFBinary, EmBPFHeader, Symbol
from embpf.instruction import TextParser, Instruction, LDDWInstruction,  LDDWDInstruction, LDDWRInstruction, LDDWBInstruction, LoadInstruction, CallInstruction, CallEncodedStackInstruction


class EmBPFFactory(ABC):
    @classmethod
    def create(cls, stream: IO[bytes], patch_function=False) -> EmBPFBinary:
        pass


class EmBPFBinarySrcFactory(EmBPFFactory):
    @classmethod
    def create(cls, stream: IO[bytes], patch_function=False) -> EmBPFBinary:
        assert not patch_function, ("Can not patch function with a EmBPFBinary"
                                    " as the source")
        return EmBPFBinary.from_bytes(stream.read())


class EmBPFELFSrcFactory(EmBPFFactory):
    TEXT = '.text'
    DATA = '.data'
    BSS = '.bss'
    RODATA = '.rodata'
    SYMBOLS = '.symtab'
    STACK_SIZES = ".stack_sizes"
    TEXT_RELOCATIONS = '.rel.text'
    DATA_RELOCATIONS = '.rel.data'
    RODATA_RELOCATIONS = '.rel.rodata'

    # https://docs.kernel.org/bpf/llvm_reloc.html
    BPF_RELOC_TYPE_NAMES = {
        0: "R_BPF_NONE",
        1: "R_BPF_64_64",
        2: "R_BPF_64_ABS64",
        3: "R_BPF_64_ABS32",
        4: "R_BPF_64_NODYLD32",
        10: "R_BPF_64_32",
    }

    @classmethod
    def create(cls, stream: IO[bytes], patch_function=False) -> EmBPFBinary:
        elffile: ELFFile = ELFFile(stream)

        section_data, bss_len, intersection_offsets = cls._process_sections(elffile)
        function_symbols: Dict[int, Symbol] = cls._process_functions(elffile, section_data[cls.RODATA])
        cls._process_relocations(elffile, section_data[cls.TEXT], intersection_offsets)
        if patch_function:
            cls._patch_function(elffile, section_data[cls.TEXT], function_symbols)
        # for name, section in section_data.items():
        #     padding_size = 8 - len(section) % 8
        #     # this is 0 if no paddig is needed
        #     padding_size %= 8
        #     section += b"\0" * padding_size
        # bss_len += (8 - bss_len % 8) % 8
        # The function info structs need to be 2 byte alligned we add the padding to the rodata
        function_offset = (sum([len(s) for s in section_data.values()]) + EmBPFHeader.b_size())
        padding_size = 2 - function_offset % 2
        padding_size %= 2
        section_data[cls.RODATA] += b"\0" * padding_size
        header = EmBPFHeader(
                magic=EmBPFHeader.get_magic(),
                version=EmBPFHeader.VERSION,
                flags=0,
                data_len=len(section_data[cls.DATA]),
                bss_len=bss_len,
                rodata_len=len(section_data[cls.RODATA]),
                text_len=len(section_data[cls.TEXT]),
                functions_len=len(function_symbols),
                )
        return EmBPFBinary(
                header=header,
                data=section_data[cls.DATA],
                rodata=section_data[cls.RODATA],
                text=section_data[cls.TEXT],
                function_symbols=list(function_symbols.values())
                )

    @classmethod
    def inst_type_from_section(cls, section: str) -> Type[LDDWInstruction]:
        if section.startswith(cls.DATA):
            return LDDWDInstruction
        elif section.startswith(cls.RODATA):
            return LDDWRInstruction
        elif section.startswith(cls.BSS):
            return LDDWBInstruction
        else:
            raise ValueError(f"Relocation in unsupported section: {section}")

    @classmethod
    def _process_sections(cls, elffile: ELFFile):
        # The bss section is zeroed. We only tack its length.
        bss_len = 0
        section_data = {
            cls.TEXT: bytearray(),
            cls.RODATA: bytearray(),
            cls.DATA: bytearray(),
        }

        # We later merg all text, rodata, data, and bss subsections into the
        # parent section. This contains the offset for each subsection
        # within the parent section
        intersection_offsets = {}

        for section in elffile.iter_sections():
            s_name: str = section.name
            logging.debug(f"Processing {s_name}")

            if section.name.startswith(cls.BSS):
                intersection_offsets[s_name] = bss_len
                bss_len += section.data_size
                continue
            target_sec: str
            for s in section_data.keys():
                if s_name.startswith(s):
                    target_sec = s
                    break
            else:
                continue
                # raise ValueError(f"Cannot find section to merge \"{s_name}\""
                #                  f" into (options: {section_data.keys()}).")

            if s_name != target_sec:
                logging.debug(f"Merging \"{s_name}\" into \"{target_sec}\"")
            intersection_offsets[s_name] = len(section_data[target_sec])
            section_data[target_sec] += section.data()
        return section_data, bss_len, intersection_offsets

    @staticmethod
    def _decode_uleb128(data, offset):
        result = 0
        shift = 0
        while True:
            byte = data[offset]
            offset += 1
            result |= (byte & 0x7F) << shift
            if (byte & 0x80) == 0:
                break
            shift += 7
        return result, offset

    @classmethod
    def _get_stack_sizes(cls, elffile) -> Dict[int, int]:
        stack_sizes: Dict[int, int] = {}
        s_size_section = elffile.get_section_by_name(cls.STACK_SIZES)

        if s_size_section is None:
            raise ValueError("No .stack_sizes section: Make sure you "
                             "compiled with -fstack-size-section")

        data = s_size_section.data()
        offset = 0
        while offset < len(data):
            address = struct.unpack_from("<Q", data, offset)[0]
            offset += 8

            stack_size, offset = cls._decode_uleb128(data, offset)
            stack_sizes[address] = stack_size
        return stack_sizes

    @classmethod
    def _process_functions(cls, elffile: ELFFile, rodata: bytes) -> Dict[int, Symbol]:
        symbols = elffile.get_section_by_name(cls.SYMBOLS)
        stack_sizes = cls._get_stack_sizes(elffile)

        function_symbols: dict[int, Symbol] = {}
        for symbol in symbols.iter_symbols():
            if symbol.entry.st_info.type != 'STT_FUNC':
                continue
            name = symbol.name
            text_offset = symbol.entry.st_value
            stack_size = stack_sizes[text_offset]
            name_offset: int = len(rodata)
            rodata += bytes(name, "UTF-8") + b"\00"
            function_symbols[text_offset] = Symbol(
                    name_offset,
                    0,
                    text_offset,
                    stack_size
                    )
            logging.debug(f"Found function \"{name}\" at 0x{text_offset:x}"
                          f"appended name to rodata at 0x{name_offset:x}")

        return function_symbols

    @classmethod
    def _process_relocations(cls, elffile: ELFFile, text: bytes,
                             intersection_offsets: Dict[str, int]):
        text_relocations = elffile.get_section_by_name(cls.TEXT_RELOCATIONS)
        data_relocations = elffile.get_section_by_name(cls.DATA_RELOCATIONS)
        rodata_relocations = elffile.get_section_by_name(cls.RODATA_RELOCATIONS)
        symbols = elffile.get_section_by_name(cls.SYMBOLS)

        if (text_relocations is not None
                and text_relocations.num_relocations() > 0):
            for relocation in text_relocations.iter_relocations():
                # logging.debug(f"Relocation: {relocation.entry}")
                entry = relocation.entry
                symbol = symbols.get_symbol(entry.r_info_sym)
                symbol_type = symbol.entry.st_info.type
                symbol_section_index = symbol.entry.st_shndx
                if symbol_section_index == "SHN_UNDEF":
                    raise RuntimeError("SHN_UNDEF section implies an external symbol being allocated. Investigate!")
                rel_type = cls.BPF_RELOC_TYPE_NAMES[entry['r_info_type']]
                target_section = elffile.get_section(symbol_section_index)

                logging.info(f"Relocation ({rel_type}) of "
                             f"{symbol.name} (type {symbol_type})."
                             )
                logging.info(f"Instruction at {hex(entry.r_offset)}"
                             f"accesses {target_section.name} + "
                             f"{symbol.entry.st_value}"
                             )
                if rel_type not in ['R_BPF_64_64', 'R_BPF_64_32']:
                    raise ValueError(f"Unsupported relocation type {rel_type}")

                cls._patch_text(
                        elffile,
                        text,
                        relocation,
                        intersection_offsets,
                        )

        if (data_relocations is not None
                and data_relocations.num_relocations() > 0):
            for relocation in data_relocations.iter_relocations():
                name = symbols.get_symbol(relocation.entry['r_info_sym']).name
                logging.critical(f"{name} requires a relocation "
                                 "in the data section!")
            raise ValueError("Relocating values in the data "
                             "section are not supported!")

        if (rodata_relocations is not None
                and rodata_relocations.num_relocations() > 0):
            for relocation in rodata_relocations.iter_relocations():
                name = symbols.get_symbol(relocation.entry['r_info_sym']).name
                logging.critical(f"{name} requires a relocation "
                                 "in the rodata section!")
            raise ValueError("Relocating values in the rodata "
                             "section are not supported!")

    @classmethod
    def _patch_text(cls, elffile: ELFFile, text: bytes, relocation: Relocation,
                    intersection_offset: Dict[str, int]):
        patched_inst: Instruction
        entry = relocation.entry
        location = entry.r_offset
        symbol = (elffile
                  .get_section_by_name(cls.SYMBOLS)
                  .get_symbol(entry.r_info_sym))

        r_type = symbol.entry.st_info.type
        if r_type in ['STT_SECTION', 'STT_OBJECT']:
            section_name = elffile.get_section(symbol.entry.st_shndx).name
            inst_type: Type[LDDWInstruction] = cls.inst_type_from_section(section_name)
            patched_inst: LDDWInstruction = inst_type.from_lddw_bytes(text[location:])
            offset = symbol.entry.st_value
            is_offset = intersection_offset[section_name]
            if is_offset > 0:
                logging.info("Symbol was located in a merged section."
                             f"Applying inter-section offset {is_offset}")
            offset += is_offset
            patched_inst.relocate(offset)

            logging.info(f"Patching LDDWInstruction at 0x{location:x} with "
                         f"{inst_type.__name__} and target 0x{offset:x}")
        elif r_type == 'STT_NOTYPE':
            assert False
        elif r_type == 'STT_FUNC':
            byte_offset = symbol.entry.st_value
            imm, rem = divmod(byte_offset - location, 8)
            assert rem == 0
            # The instruction index points to the next instruction already
            imm -= 1
            patched_inst: CallInstruction = CallInstruction.from_bytes(text[location:], location)
            logging.info(f"Patching CallInstruction at 0x{location:x} with new"
                         f" immediate {imm}")
            patched_inst.relocate(imm)
        else:
            raise ValueError(f"Unknown st_info {symbol.entry.st_info.type}")
        assert patched_inst is not None
        text[location:location + patched_inst.b_size()] = patched_inst.bytes()

    @classmethod
    def _patch_function(cls, elffile: ELFFile, text: bytes, function_symbols: Dict[int, Symbol]):
        for ins in TextParser.instruction_generator(text):
            if not isinstance(ins, CallInstruction) or ins.src_register != 1:
                continue
            target_address = ins.address + (ins.immediate + 1) * 8
            stack_size: int = function_symbols[target_address].stack_size
            patched_inst = CallEncodedStackInstruction.from_call(ins, stack_size)
            text[patched_inst.address:patched_inst.address + patched_inst.b_size():] = patched_inst.bytes()
