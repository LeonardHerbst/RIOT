import struct

from dataclasses import dataclass
from typing import List, Dict, Tuple

from embpf.instruction import TextParser, Instruction


@dataclass
class Symbol:
    name_offset: int
    flags: int
    location_offset: int
    stack_size: int

    FMT = "<HHHH"

    @classmethod
    def b_size(cls):
        return struct.calcsize(cls.FMT)

    @classmethod
    def from_bytes(cls, binary: bytes):
        return cls(*struct.unpack_from(cls.FMT, binary))

    def to_bytes(self) -> bytes:
        return struct.pack(
                self.FMT,
                self.name_offset,
                self.flags,
                self.location_offset,
                self.stack_size
                )

    def get_name(self, rodata: bytes) -> str:
        # The function names a stored as null terminated string at the end
        # of the read-only data
        return rodata[self.name_offset:].split(b"\00")[0].decode("UTF-8")


@dataclass
class EmBPFHeader:
    magic: int
    version: int
    flags: int
    data_len: int
    bss_len: int
    rodata_len: int
    text_len: int
    functions_len: int

    FMT = "<IIIIIIII"
    VERSION = 0

    @classmethod
    def b_size(cls) -> int:
        return struct.calcsize(cls.FMT)

    @staticmethod
    def get_magic():
        return int.from_bytes(b"mBPF", "little")

    @classmethod
    def from_bytes(cls, binary: bytes):
        header_struct = struct.unpack_from(cls.FMT, binary)
        assert header_struct[0] == cls.get_magic(), "Magic value is incorrect!"
        return cls(*header_struct)

    def to_bytes(self) -> bytes:
        return struct.pack(
                self.FMT,
                self.magic,
                self.version,
                self.flags,
                self.data_len,
                self.bss_len,
                self.rodata_len,
                self.text_len,
                self.functions_len,
                )

    def dump(self):
        print(f"Magic:\t\t{hex(self.magic)}\n"
              f"Version:\t{self.version}\n"
              f"flags:\t\t{bin(self.flags)}\n"
              f"Data length:\t{self.data_len} B\n"
              f"Bss length:\t{self.bss_len} B\n"
              f"RoData length:\t{self.rodata_len} B\n"
              f"Text length:\t{self.text_len} B\n"
              f"No. functions:\t{self.functions_len}\n"
              )


class EmBPFBinary:
    def __init__(self, header: EmBPFHeader, data: bytes, rodata: bytes, text: bytes, function_symbols: List[Symbol]):
        self.header = header
        self.data = data
        self.rodata = rodata
        self.text = text
        self.function_symbols: List[Symbol] = function_symbols

    @classmethod
    def from_bytes(cls, binary: bytes):
        header: EmBPFHeader = EmBPFHeader.from_bytes(binary)
        offset: int = EmBPFHeader.b_size()
        # Header | Data | Rodata | Text | Symbols
        fmt = f"<{header.data_len}s{header.rodata_len}s{header.text_len}s"
        data, rodata, text = struct.unpack_from(fmt, binary, offset)

        offset += struct.calcsize(fmt)
        function_symbols: List[Symbol] = []
        for i in range(offset, len(binary), Symbol.b_size()):
            function_symbols.append(Symbol.from_bytes(binary[i:]))

        return cls(header, data, rodata, text, function_symbols)

    def to_bytes(self) -> bytes:
        out: bytes = bytearray(self.header.to_bytes())
        out += self.data
        out += self.rodata
        out += self.text
        for symbol in self.function_symbols:
            out += symbol.to_bytes()
        return out

    def instructions(self) -> List[Instruction]:
        return TextParser.parse_text(self.text)

    def dump(self):
        functions: Dict[int, Tuple[str, int]] = self._parse_function_names()
        instruction_list: List[Instruction] = self.instructions()

        self.header.dump()

        function_names = "\n\t".join([n for n, _ in functions.values()])
        print(f"functions: \n\t{function_names}")

        print("data:")
        print("".join(data for data in EmBPFBinary.obj_hexstr(self.data)))

        print("bss:")
        print("".join(
            data for data in EmBPFBinary.obj_hexstr([0] * self.header.bss_len)))

        print("rodata:")
        print("".join(data for data in EmBPFBinary.obj_hexstr(self.rodata)))

        print("text:")
        for instr in instruction_list:
            if instr.address in functions:
                name, stack = functions[instr.address]
                print(f"<{name}> ({stack} Bytes Stack):")
            print(instr.full_print())

    def _parse_function_names(self) -> Dict[int, Tuple[str, int]]:
        ret: Dict[int, str] = {}
        for symbol in self.function_symbols:
            ret[symbol.location_offset] = \
                    (symbol.get_name(self.rodata), symbol.stack_size)
        return ret

    @staticmethod
    def obj_hexstr(bstr: bytes, line_len: int = 8):
        for i in range(0, len(bstr), line_len):
            line = " ".join(f"0x{b:0>2x}" for b in bstr[i:i + line_len])
            yield f"{i:>5}: {line}\n"
