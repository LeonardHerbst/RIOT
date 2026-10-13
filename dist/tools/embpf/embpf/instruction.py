import struct
import logging
from dataclasses import dataclass
from typing import List, Dict, Any, ClassVar

# Translation dics

BPF_CLASS = {
    0x00: "LD",     # load
    0x01: "LDX",    # load from register
    0x02: "ST",     # store immediate
    0x03: "STX",    # store register
    0x04: "ALU32",    # 32-bit arithmetic/logic
    0x05: "JMP64",    # 64-bit jump
    0x06: "JMP32",  # 32-bit jump
    0x07: "ALU64",  # 64-bit arithmetic/logic
}

BPF_SIZE = {
    0x00: "W (32 bit)",   # 32-bit word   (4 bytes)
    0x01: "H (16 bit)",   # 16-bit half-word (2 bytes)
    0x02: "B (8 bit)",   # 8-bit byte    (1 byte)
    0x03: "DW (64 bit)",  # 64-bit double-word (8 bytes)
}

BPF_MODE = {
    0x00: "IMM",     # 64-bit immediate
    0x01: "ABS",     # legacy BPF absolute packet access
    0x02: "IND",     # legacy BPF indirect packet access
    0x03: "MEM",     # regular memory load/store
    0x04: "MEMSX",   # sign-extending memory load
    0x05: "CUSTOM",  # Custom load instructions
    0x06: "ATOMIC",  # atomic memory operation
}

BPF_SOURCE = {
    0x00: "Imm",  # 32-bit immediate constant
    0x01: "Reg",  # src_reg register
}

# Custom Opcodes
LDDW_OPCODE = 0x18
LDDWD_OPCODE = 0xB8
LDDWR_OPCODE = 0xB0
LDDWB_OPCODE = 0xA8

CALLES_OPCODE = 0x8D


@dataclass
class Instruction:
    registers: int
    offset: int
    immediate: int
    address: int = 0

    FMT = "<BBhi"
    OPCODE = 0x00
    OP_CLASS: ClassVar[int | None] = None

    @classmethod
    def from_bytes(cls, binary: bytes, address: int):
        opcode, registers, offset, immediate = \
                struct.unpack_from(cls.FMT, binary)
        if cls.opcode() != opcode:
            raise ValueError("Opcode not matching expected "
                             f"{hex(cls.opcode())} but found {hex(opcode)}")
        logging.debug(f"Creating instruction {hex(opcode)} with {registers}, "
                      f"{offset}, {immediate}")
        return cls(registers, offset, immediate, address)

    @classmethod
    def opcode(cls) -> int:
        return cls.OPCODE

    @classmethod
    def b_size(cls) -> int:
        return struct.calcsize(cls.FMT)

    @property
    def src_register(self) -> int:
        return (self.registers & 0xf0) >> 4

    @property
    def dst_register(self) -> int:
        return (self.registers & 0x0f)

    def get_stats(self) -> Dict[str, Any]:
        pass

    def asm_print(self):
        return f"r{self.dst_register} = r{self.src_register}"

    def bytes(self) -> bytes:
        return struct.pack(
                self.FMT,
                self.OPCODE,
                self.registers,
                self.offset,
                self.immediate
                )

    def full_print(self):
        hexdump = " ".join(f"{b:0>2x}" for b in self.bytes())
        asm = self.asm_print()
        return f"{str(self.address // 8): >7}:\t{hexdump} {asm}"


class LoadAndStoreInstruction(Instruction):
    OP_SIZE: ClassVar[int | None] = None
    OP_MODE: ClassVar[int | None] = None

    def build_opcode(self) -> int:
        opcode = 0
        opcode |= self.OP_CLASS
        opcode |= self.OP_SIZE << 3
        opcode |= self.OP_MODE << 5
        assert self.OPCODE == opcode, f"Wrong opcode for {type(self).__name__} (got 0b{opcode:8b} but expected 0b{self.OPCODE:8b})"
        return opcode

    @property
    def category(self) -> str:
        return "LoadAndStore"

    def get_stats(self) -> Dict[str, Any]:
        assert self.OP_CLASS is not None, f"{type(self).__name__} as no OP_CLASS"
        assert self.OP_MODE is not None, f"{type(self).__name__} as no OP_MODE"
        assert self.OP_SIZE is not None, f"{type(self).__name__} as no OP_SIZE"
        _ = self.build_opcode()
        return {
            "category": "Load and Store",
            "class": BPF_CLASS[self.OP_CLASS],
            "mode": BPF_MODE[self.OP_MODE],
            "size": BPF_SIZE[self.OP_SIZE],
            "name": type(self).__name__[:-11]
        }


class ArithmeticAndJumpInstruction(Instruction):
    OP_CODE: ClassVar[int | None] = None
    OP_SOURCE: ClassVar[int | None] = None
    BIT_WIDTH: ClassVar[int | None] = None

    def build_opcode(self) -> int:
        opcode = 0
        opcode |= self.OP_CLASS
        opcode |= self.OP_SOURCE << 3
        opcode |= self.OP_CODE << 4
        assert self.OPCODE == opcode, f"Wrong opcode for {type(self).__name__} (got 0b{opcode:8b} but expected 0b{self.OPCODE:8b})"
        return opcode

    def get_stats(self) -> Dict[str, Any]:
        assert self.OP_CLASS is not None, f"{type(self).__name__} as no OP_CLASS"
        assert self.OP_SOURCE is not None, f"{type(self).__name__} as no OP_SOURCE"
        assert self.OP_CODE is not None, f"{type(self).__name__} as no OP_CODE"
        _ = self.build_opcode()
        return {
            "category": "Arithmetic and Jump",
            "class": BPF_CLASS[self.OP_CLASS],
            "bit_width": self.BIT_WIDTH,
            "source": BPF_SOURCE[self.OP_SOURCE],
            "name": type(self).__name__[:-11]
        }


class AluInstruction(ArithmeticAndJumpInstruction):
    OP_CLASS: ClassVar[int | None] = 0x7
    OP_SOURCE: ClassVar[int | None] = 0x1
    OPERAND = '+'
    BIT_WIDTH: ClassVar[int | None] = 64

    @property
    def operand(self):
        return self.OPERAND

    def asm_print(self):
        return f"r{self.dst_register} {self.operand}= r{self.src_register}"


class Alu32BitInstruction(AluInstruction):
    OP_CLASS: ClassVar[int | None] = 0x4
    BIT_WIDTH: ClassVar[int | None] = 32

    def asm_print(self):
        return f"w{self.dst_register} {self.operand}= w{self.src_register}"


class AluImmInstruction(AluInstruction):
    OP_SOURCE: ClassVar[int | None] = 0x0

    def asm_print(self):
        return f"r{self.dst_register} {self.operand}= {self.immediate}"


class Alu32BitImmInstruction(Alu32BitInstruction):
    OP_SOURCE: ClassVar[int | None] = 0x0

    def asm_print(self):
        return f"w{self.dst_register} {self.operand}= {self.immediate}"


# --- ADD ---
class AddInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x00


class AddImmInstruction(AddInstructionCode, AluImmInstruction):
    OPERAND = '+'
    OPCODE = 0x07


class Add32BitImmInstruction(AddInstructionCode, Alu32BitImmInstruction):
    OPERAND = '+'
    OPCODE = 0x04


class AddInstruction(AddInstructionCode, AluInstruction):
    OPERAND = '+'
    OPCODE = 0x0f


class Add32BitInstruction(AddInstructionCode, Alu32BitInstruction):
    OPERAND = '+'
    OPCODE = 0x0c


# --- SUB ---
class SubInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x01


class SubImmInstruction(SubInstructionCode, AluImmInstruction):
    OPERAND = '-'
    OPCODE = 0x17


class Sub32BitImmInstruction(SubInstructionCode, Alu32BitImmInstruction):
    OPERAND = '-'
    OPCODE = 0x14


class SubInstruction(SubInstructionCode, AluInstruction):
    OPERAND = '-'
    OPCODE = 0x1f


class Sub32BitInstruction(SubInstructionCode, Alu32BitInstruction):
    OPERAND = '-'
    OPCODE = 0x1c


# --- MUL ---
class MulInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x02


class MulImmInstruction(MulInstructionCode, AluImmInstruction):
    OPERAND = '*'
    OPCODE = 0x27


class Mul32BitImmInstruction(MulInstructionCode, Alu32BitImmInstruction):
    OPERAND = '*'
    OPCODE = 0x24


class MulInstruction(MulInstructionCode, AluInstruction):
    OPERAND = '*'
    OPCODE = 0x2f


class Mul32BitInstruction(MulInstructionCode, Alu32BitInstruction):
    OPERAND = '*'
    OPCODE = 0x2c


# --- DIV ---
class DivInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x03


class DivImmInstruction(DivInstructionCode, AluImmInstruction):
    OPERAND = '/'
    OPCODE = 0x37


class Div32BitImmInstruction(DivInstructionCode, Alu32BitImmInstruction):
    OPERAND = '/'
    OPCODE = 0x34


class DivInstruction(DivInstructionCode, AluInstruction):
    OPERAND = '/'
    OPCODE = 0x3f


class Div32BitInstruction(DivInstructionCode, Alu32BitInstruction):
    OPERAND = '/'
    OPCODE = 0x3c


# --- OR ---
class OrInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x04


class OrImmInstruction(OrInstructionCode, AluImmInstruction):
    OPERAND = '|'
    OPCODE = 0x47


class Or32BitImmInstruction(OrInstructionCode, Alu32BitImmInstruction):
    OPERAND = '|'
    OPCODE = 0x44


class OrInstruction(OrInstructionCode, AluInstruction):
    OPERAND = '|'
    OPCODE = 0x4f


class Or32BitInstruction(OrInstructionCode, Alu32BitInstruction):
    OPERAND = '|'
    OPCODE = 0x4c


# --- AND ---
class AndInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x05


class AndImmInstruction(AndInstructionCode, AluImmInstruction):
    OPERAND = '&'
    OPCODE = 0x57


class And32BitImmInstruction(AndInstructionCode, Alu32BitImmInstruction):
    OPERAND = '&'
    OPCODE = 0x54


class AndInstruction(AndInstructionCode, AluInstruction):
    OPERAND = '&'
    OPCODE = 0x5f


class And32BitInstruction(AndInstructionCode, Alu32BitInstruction):
    OPERAND = '&'
    OPCODE = 0x5c


# --- LSH (Left Shift) ---
class LSHInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x06


class LSHImmInstruction(LSHInstructionCode, AluImmInstruction):
    OPERAND = '<<'
    OPCODE = 0x67


class LSH32BitImmInstruction(LSHInstructionCode, Alu32BitImmInstruction):
    OPERAND = '<<'
    OPCODE = 0x64


class LSHInstruction(LSHInstructionCode, AluInstruction):
    OPERAND = '<<'
    OPCODE = 0x6f


class LSH32BitInstruction(LSHInstructionCode, Alu32BitInstruction):
    OPERAND = '<<'
    OPCODE = 0x6c


# --- RSH (Right Shift) ---
class RSHInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x07


class RSHImmInstruction(RSHInstructionCode, AluImmInstruction):
    OPERAND = '>>'
    OPCODE = 0x77


class RSH32BitImmInstruction(RSHInstructionCode, Alu32BitImmInstruction):
    OPERAND = '>>'
    OPCODE = 0x74


class RSHInstruction(RSHInstructionCode, AluInstruction):
    OPERAND = '>>'
    OPCODE = 0x7f


class RSH32BitInstruction(RSHInstructionCode, Alu32BitInstruction):
    OPERAND = '>>'
    OPCODE = 0x7c


# --- NEG ---
class NEGInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x08
    # "The NEG instruction is only defined when the source bit is clear (K)."
    OP_SOURCE: ClassVar[int | None] = 0x00


class NegInstruction(NEGInstructionCode, AluInstruction):
    OPERAND = '~'
    OPCODE = 0x87


class Neg32BitInstruction(NEGInstructionCode, Alu32BitInstruction):
    OPERAND = '~'
    OPCODE = 0x84


# --- MOD ---
class MODInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x09


class ModImmInstruction(MODInstructionCode, AluImmInstruction):
    OPERAND = '%'
    OPCODE = 0x97


class Mod32BitImmInstruction(MODInstructionCode, Alu32BitImmInstruction):
    OPERAND = '%'
    OPCODE = 0x94


class ModInstruction(MODInstructionCode, AluInstruction):
    OPERAND = '%'
    OPCODE = 0x9f


class Mod32BitInstruction(MODInstructionCode, Alu32BitInstruction):
    OPERAND = '%'
    OPCODE = 0x9c


# --- XOR ---
class XORInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x0a


class XorImmInstruction(XORInstructionCode, AluImmInstruction):
    OPERAND = '^'
    OPCODE = 0xa7


class Xor32BitImmInstruction(XORInstructionCode, Alu32BitImmInstruction):
    OPERAND = '^'
    OPCODE = 0xa4


class XorInstruction(XORInstructionCode, AluInstruction):
    OPERAND = '^'
    OPCODE = 0xaf


class Xor32BitInstruction(XORInstructionCode, Alu32BitInstruction):
    OPERAND = '^'
    OPCODE = 0xac


# --- MOV ---
class MOVInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x0b


class MovImmInstruction(MOVInstructionCode, AluImmInstruction):
    OPERAND = ''
    OPCODE = 0xb7


class Mov32BitImmInstruction(MOVInstructionCode, Alu32BitImmInstruction):
    OPERAND = ''
    OPCODE = 0xb4


class MovInstruction(MOVInstructionCode, AluInstruction):
    OPERAND = ''
    OPCODE = 0xbf


class Mov32BitInstruction(MOVInstructionCode, Alu32BitInstruction):
    OPERAND = ''
    OPCODE = 0xbc


# --- ARSH (Arithmetic Right Shift) ---
class ARSHInstructionCode:
    OP_CODE: ClassVar[int | None] = 0x0c


class ARSHImmInstruction(ARSHInstructionCode, AluImmInstruction):
    OPERAND = 's>>'
    OPCODE = 0xc7


class ARSH32BitImmInstruction(ARSHInstructionCode, Alu32BitImmInstruction):
    OPERAND = 's>>'
    OPCODE = 0xc4


class ARSHInstruction(ARSHInstructionCode, AluInstruction):
    OPERAND = 's>>'
    OPCODE = 0xcf


class ARSH32BitInstruction(ARSHInstructionCode, Alu32BitInstruction):
    OPERAND = 's>>'
    OPCODE = 0xcc


# --- Memory Instructions ---
class MemInstruction(LoadAndStoreInstruction):
    SIZE_STRING = {
        0b00: "uint32_t",
        0b01: "uint16_t",
        0b10: "uint8_t",
        0b11: "uint64_t"
    }

    @property
    def size(self):
        return (self.OPCODE & 0x18) >> 3

    @property
    def size_str(self):
        return self.SIZE_STRING.get(self.size, "UNKNOWN SIZE")

    @property
    def sign_str(self):
        return " + " if self.offset >= 0 else " - "


class LoadInstruction(MemInstruction):
    OP_CLASS: ClassVar[int | None] = 0x00
    OP_MODE: ClassVar[int | None] = 0x00

    def asm_print(self):
        return f"r{self.dst_register} = {self.immediate}"


class LoadXInstruction(MemInstruction):
    OP_CLASS: ClassVar[int | None] = 0x01
    OP_MODE: ClassVar[int | None] = 0x03

    def asm_print(self):
        return (f"r{self.dst_register} = *({self.size_str}*)"
                f"(r{self.src_register}{self.sign_str}{abs(self.offset)})")


class StoreXInstruction(MemInstruction):
    OP_CLASS: ClassVar[int | None] = 0x03
    OP_MODE: ClassVar[int | None] = 0x03

    def asm_print(self):
        return (f"*({self.size_str}*)(r{self.dst_register}{self.sign_str}"
                f"{abs(self.offset)}) = r{self.src_register}")


class StoreInstruction(MemInstruction):
    OP_CLASS: ClassVar[int | None] = 0x03
    OP_MODE: ClassVar[int | None] = 0x00

    def asm_print(self):
        return (f"*({self.size_str}*)(r{self.dst_register}{self.sign_str}"
                f"{abs(self.offset)}) = {self.immediate}")


class LDDWInstruction(LoadInstruction):
    OP_CLASS: ClassVar[int | None] = 0x00
    OP_SIZE: ClassVar[int | None] = 0x03
    OP_MODE: ClassVar[int | None] = 0x00

    OPCODE = LDDW_OPCODE
    FMT = "<BBhIBBhI"

    def __init__(self, registers, offset, immediate_l, immediate_h, address=0):
        self.immediate_l = immediate_l
        self.immediate_h = immediate_h
        immediate = (self.immediate_h << 32) + self.immediate_l
        super().__init__(registers, offset, immediate, address)

    @classmethod
    def from_bytes(cls, instruction: bytes, address=0):
        if cls.opcode() != instruction[0]:
            raise RuntimeError("Opcode not matching expected, got "
                               f"{instruction[0]}, expected {cls.opcode()}")
        return cls.from_lddw_bytes(instruction, address)

    @classmethod
    def from_lddw_bytes(cls, instruction: bytes, address=0):
        # This differs from from_bytes in that it creates a LDDWD/R/B
        # instruction from the bytes of any LDDW type instruction
        opcode, registers, offset, immediate_l, _, _, _, immediate_h = \
            struct.unpack_from(cls.FMT, instruction)
        return cls(registers, offset, immediate_l, immediate_h, address)

    def bytes(self):
        return struct.pack(
                self.FMT,
                self.OPCODE,
                self.registers,
                self.offset,
                self.immediate_l,
                0,
                0,
                0,
                self.immediate_h
                )

    def relocate(self, immedate_offset: int) -> None:
        self.immediate += immedate_offset
        self.immediate_l = self.immediate & 0xffffffff
        self.immediate_h = self.immediate >> 32


class CustomLoadInstruction(LDDWInstruction):
    OP_CLASS: ClassVar[int | None] = 0x00
    # New mode 0x05 (unused this far)
    OP_MODE: ClassVar[int | None] = 0x05
    # The SIZE specifies the segment from which we load (3 -> data, 2 -> rodata, 1-> bss, 0 -> unused)
    OP_SIZE: ClassVar[int | None] = None


class LDDWDInstruction(CustomLoadInstruction):
    # The SIZE specifies the segment we load from for the new load moad. 3 = .data
    OP_SIZE: ClassVar[int | None] = 0x3
    OPCODE = LDDWD_OPCODE

    def asm_print(self):
        return f"r{self.dst_register} = {self.immediate} + .data"


class LDDWRInstruction(CustomLoadInstruction):
    # The SIZE specifies the segment we load from for the new load moad. 2 = .rodata
    OP_SIZE: ClassVar[int | None] = 0x2
    OPCODE = LDDWR_OPCODE

    def asm_print(self):
        return f"r{self.dst_register} = {self.immediate} + .rodata"


class LDDWBInstruction(CustomLoadInstruction):
    # The SIZE specifies the segment we load from for the new load moad. 1 = .bss
    OP_SIZE: ClassVar[int | None] = 0x1
    OPCODE = LDDWB_OPCODE

    def asm_print(self):
        return f"r{self.dst_register} = {self.immediate} + .bss"


class LDXDWInstruction(LoadXInstruction):
    OP_SIZE: ClassVar[int | None] = 0x03
    OPCODE = 0x79


class LDXWInstruction(LoadXInstruction):
    OP_SIZE: ClassVar[int | None] = 0x00
    OPCODE = 0x61


class LDXHInstruction(LoadXInstruction):
    OP_SIZE: ClassVar[int | None] = 0x01
    OPCODE = 0x69


class LDXBInstruction(LoadXInstruction):
    OP_SIZE: ClassVar[int | None] = 0x02
    OPCODE = 0x71


class STXDWInstruction(StoreXInstruction):
    OP_SIZE: ClassVar[int | None] = 0x03
    OPCODE = 0x7b


class STXWInstruction(StoreXInstruction):
    OP_SIZE: ClassVar[int | None] = 0x00
    OPCODE = 0x63


class STXHInstruction(StoreXInstruction):
    OP_SIZE: ClassVar[int | None] = 0x01
    OPCODE = 0x6b


class STXBInstruction(StoreXInstruction):
    OP_SIZE: ClassVar[int | None] = 0x02
    OPCODE = 0x73


class STDWInstruction(StoreInstruction):
    OP_SIZE: ClassVar[int | None] = 0x03
    OPCODE = 0x7a


class STWInstruction(StoreInstruction):
    OP_SIZE: ClassVar[int | None] = 0x00
    OPCODE = 0x62


class STHInstruction(StoreInstruction):
    OP_SIZE: ClassVar[int | None] = 0x01
    OPCODE = 0x6a


class STBInstruction(StoreInstruction):
    OP_SIZE: ClassVar[int | None] = 0x02
    OPCODE = 0x72


# --- Branch Instructions ---
class BranchInstruction(ArithmeticAndJumpInstruction):
    OP_CLASS: ClassVar[int | None] = 0x05
    OP_SOURCE: ClassVar[int | None] = 0x01
    OPERAND = '=='
    BIT_WIDTH: ClassVar[int | None] = 64
    target: Instruction | None = None
    offset: int

    def set_target(self, target: Instruction):
        self.target = target
        self.offset = (self.target.address - self.address - self.b_size()) // 8

    @property
    def operand(self):
        return self.OPERAND

    def asm_print(self):
        return (f"if r{self.dst_register} {self.operand} r{self.src_register} "
                f"goto {self.offset}")


class BranchImmInstruction(BranchInstruction):
    OP_SOURCE: ClassVar[int | None] = 0x00

    def asm_print(self):
        return (f"if r{self.dst_register} {self.operand} {self.immediate} "
                f"goto {self.offset}")


class AlwaysBranchInstruction(BranchInstruction):
    # Always branch needs to be {JA, K, JMP}. K means SOURCE = 0
    OP_SOURCE: ClassVar[int | None] = 0x00
    OP_CODE: ClassVar[int | None] = 0x00
    OPCODE = 0x05

    def asm_print(self):
        return f"goto {self.offset}"


class EqBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x01
    OPERAND = "=="
    OPCODE = 0x1d


class EqBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x01
    OPERAND = "=="
    OPCODE = 0x15


class GtBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x02
    OPERAND = ">"
    OPCODE = 0x2d


class GtBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x02
    OPERAND = ">"
    OPCODE = 0x25


class GeBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x03
    OPERAND = ">="
    OPCODE = 0x3d


class GeBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x03
    OPERAND = ">="
    OPCODE = 0x35


class LtBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x0a
    OPERAND = "<"
    OPCODE = 0xad


class LtBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x0a
    OPERAND = "<"
    OPCODE = 0xa5


class LeBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x0b
    OPERAND = "<="
    OPCODE = 0xbd


class LeBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x0b
    OPERAND = "<="
    OPCODE = 0xb5


class SetBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x04
    OPERAND = "&"
    OPCODE = 0x4d


class SetBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x04
    OPERAND = "&"
    OPCODE = 0x45


class NeBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x05
    OPERAND = "!="
    OPCODE = 0x5d


class NeBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x05
    OPERAND = "!="
    OPCODE = 0x55


class SGtBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x06
    OPERAND = "s>"
    OPCODE = 0x6d


class SGtBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x06
    OPERAND = "s>"
    OPCODE = 0x65


class SGeBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x07
    OPERAND = "s>="
    OPCODE = 0x7d


class SGeBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x07
    OPERAND = "s>="
    OPCODE = 0x75


class SLtBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x0c
    OPERAND = "s<"
    OPCODE = 0xcd


class SLtBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x0c
    OPERAND = "s<"
    OPCODE = 0xc5


class SLeBranchInstruction(BranchInstruction):
    OP_CODE: ClassVar[int | None] = 0x0d
    OPERAND = "s<="
    OPCODE = 0xdd


class SLeBranchImmInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x0d
    OPERAND = "s<="
    OPCODE = 0xd5


class CallInstruction(BranchImmInstruction):
    OP_CODE: ClassVar[int | None] = 0x08
    OPCODE = 0x85

    def asm_print(self):
        return f"Call {self.immediate}"

    def relocate(self, immedate: int):
        self.immediate = immedate


class CallEncodedStackInstruction(CallInstruction):
    # Custom operation that holds the required stack for the funciton call in the offset
    OP_CODE: ClassVar[int | None] = 0x08
    OP_SOURCE: ClassVar[int | None] = 0x01
    OPCODE = CALLES_OPCODE

    @classmethod
    def from_call(cls, call_inst: CallInstruction, stack_size: int):
        return cls(
                registers=call_inst.registers,
                immediate=call_inst.immediate,
                offset=stack_size,
                address=call_inst.address
                )

    def asm_print(self):
        return f"Call {self.immediate} ({self.offset} Bytes stack)"


class ReturnInstruction(ArithmeticAndJumpInstruction):
    OP_CLASS: ClassVar[int | None] = 0x05
    OP_SOURCE: ClassVar[int | None] = 0x00
    OP_CODE: ClassVar[int | None] = 0x09
    BIT_WIDTH: ClassVar[int | None] = 64
    OPCODE = 0x95

    def asm_print(self):
        return "Return r0"


class TextParser:
    INSTRUCTIONS: Dict[int, Instruction] = {
        # ALU 64-bit register
        AddInstruction.OPCODE: AddInstruction,
        SubInstruction.OPCODE: SubInstruction,
        MulInstruction.OPCODE: MulInstruction,
        DivInstruction.OPCODE: DivInstruction,
        OrInstruction.OPCODE: OrInstruction,
        AndInstruction.OPCODE: AndInstruction,
        LSHInstruction.OPCODE: LSHInstruction,
        RSHInstruction.OPCODE: RSHInstruction,
        NegInstruction.OPCODE: NegInstruction,
        ModInstruction.OPCODE: ModInstruction,
        XorInstruction.OPCODE: XorInstruction,
        MovInstruction.OPCODE: MovInstruction,
        ARSHInstruction.OPCODE: ARSHInstruction,

        # ALU 64-bit immediate
        AddImmInstruction.OPCODE: AddImmInstruction,
        SubImmInstruction.OPCODE: SubImmInstruction,
        MulImmInstruction.OPCODE: MulImmInstruction,
        DivImmInstruction.OPCODE: DivImmInstruction,
        OrImmInstruction.OPCODE: OrImmInstruction,
        AndImmInstruction.OPCODE: AndImmInstruction,
        LSHImmInstruction.OPCODE: LSHImmInstruction,
        RSHImmInstruction.OPCODE: RSHImmInstruction,
        ModImmInstruction.OPCODE: ModImmInstruction,
        XorImmInstruction.OPCODE: XorImmInstruction,
        MovImmInstruction.OPCODE: MovImmInstruction,
        ARSHImmInstruction.OPCODE: ARSHImmInstruction,

        # ALU 32-bit register
        Add32BitInstruction.OPCODE: Add32BitInstruction,
        Sub32BitInstruction.OPCODE: Sub32BitInstruction,
        Mul32BitInstruction.OPCODE: Mul32BitInstruction,
        Div32BitInstruction.OPCODE: Div32BitInstruction,
        Or32BitInstruction.OPCODE: Or32BitInstruction,
        And32BitInstruction.OPCODE: And32BitInstruction,
        LSH32BitInstruction.OPCODE: LSH32BitInstruction,
        RSH32BitInstruction.OPCODE: RSH32BitInstruction,
        Neg32BitInstruction.OPCODE: Neg32BitInstruction,
        Mod32BitInstruction.OPCODE: Mod32BitInstruction,
        Xor32BitInstruction.OPCODE: Xor32BitInstruction,
        Mov32BitInstruction.OPCODE: Mov32BitInstruction,
        ARSH32BitInstruction.OPCODE: ARSH32BitInstruction,

        # ALU 32-bit immediate
        Add32BitImmInstruction.OPCODE: Add32BitImmInstruction,
        Sub32BitImmInstruction.OPCODE: Sub32BitImmInstruction,
        Mul32BitImmInstruction.OPCODE: Mul32BitImmInstruction,
        Div32BitImmInstruction.OPCODE: Div32BitImmInstruction,
        Or32BitImmInstruction.OPCODE: Or32BitImmInstruction,
        And32BitImmInstruction.OPCODE: And32BitImmInstruction,
        LSH32BitImmInstruction.OPCODE: LSH32BitImmInstruction,
        RSH32BitImmInstruction.OPCODE: RSH32BitImmInstruction,
        Mod32BitImmInstruction.OPCODE: Mod32BitImmInstruction,
        Xor32BitImmInstruction.OPCODE: Xor32BitImmInstruction,
        Mov32BitImmInstruction.OPCODE: Mov32BitImmInstruction,
        ARSH32BitImmInstruction.OPCODE: ARSH32BitImmInstruction,

        # Memory
        LDDWInstruction.OPCODE: LDDWInstruction,
        LDXDWInstruction.OPCODE: LDXDWInstruction,
        LDXWInstruction.OPCODE: LDXWInstruction,
        LDXHInstruction.OPCODE: LDXHInstruction,
        LDXBInstruction.OPCODE: LDXBInstruction,
        STXDWInstruction.OPCODE: STXDWInstruction,
        STXWInstruction.OPCODE: STXWInstruction,
        STXHInstruction.OPCODE: STXHInstruction,
        STXBInstruction.OPCODE: STXBInstruction,
        STDWInstruction.OPCODE: STDWInstruction,
        STWInstruction.OPCODE: STWInstruction,
        STHInstruction.OPCODE: STHInstruction,
        STBInstruction.OPCODE: STBInstruction,

        # Branch
        AlwaysBranchInstruction.OPCODE: AlwaysBranchInstruction,
        EqBranchInstruction.OPCODE: EqBranchInstruction,
        EqBranchImmInstruction.OPCODE: EqBranchImmInstruction,
        GtBranchInstruction.OPCODE: GtBranchInstruction,
        GtBranchImmInstruction.OPCODE: GtBranchImmInstruction,
        GeBranchInstruction.OPCODE: GeBranchInstruction,
        GeBranchImmInstruction.OPCODE: GeBranchImmInstruction,
        LtBranchInstruction.OPCODE: LtBranchInstruction,
        LtBranchImmInstruction.OPCODE: LtBranchImmInstruction,
        LeBranchInstruction.OPCODE: LeBranchInstruction,
        LeBranchImmInstruction.OPCODE: LeBranchImmInstruction,
        SetBranchInstruction.OPCODE: SetBranchInstruction,
        SetBranchImmInstruction.OPCODE: SetBranchImmInstruction,
        NeBranchInstruction.OPCODE: NeBranchInstruction,
        NeBranchImmInstruction.OPCODE: NeBranchImmInstruction,
        SGtBranchInstruction.OPCODE: SGtBranchInstruction,
        SGtBranchImmInstruction.OPCODE: SGtBranchImmInstruction,
        SGeBranchInstruction.OPCODE: SGeBranchInstruction,
        SGeBranchImmInstruction.OPCODE: SGeBranchImmInstruction,
        SLtBranchInstruction.OPCODE: SLtBranchInstruction,
        SLtBranchImmInstruction.OPCODE: SLtBranchImmInstruction,
        SLeBranchInstruction.OPCODE: SLeBranchInstruction,
        SLeBranchImmInstruction.OPCODE: SLeBranchImmInstruction,

        # Call and Return
        CallInstruction.OPCODE: CallInstruction,
        CallEncodedStackInstruction.OPCODE: CallEncodedStackInstruction,
        ReturnInstruction.OPCODE: ReturnInstruction,

        # Custom emBPF
        LDDWDInstruction.OPCODE: LDDWDInstruction,
        LDDWRInstruction.OPCODE: LDDWRInstruction,
        LDDWBInstruction.OPCODE: LDDWBInstruction
    }

    @classmethod
    def instruction_from_bytes(cls, instruction: bytes, address: int) -> Instruction:
        opcode = instruction[0]
        instr_type = cls.INSTRUCTIONS.get(opcode, None)
        assert instr_type is not None, f"Unknown instruction with opcode {opcode:x}"
        return instr_type.from_bytes(instruction, address)

    @classmethod
    def instruction_generator(cls, text: bytes):
        offset: int = 0
        while offset < len(text):
            ins = cls.instruction_from_bytes(text[offset:], offset)
            yield ins
            offset += ins.b_size()

    @classmethod
    def parse_text(cls, text: bytes) -> List[Instruction]:
        instructions: Dict[int, Instruction] = {}
        for ins in cls.instruction_generator(text):
            logging.debug(f"{type(ins).__name__} at 0x{ins.address:x}: {ins}")
            if ins != instructions.setdefault(ins.address, ins):
                raise RuntimeError(f"Two instruction with address {ins.address}")
        for address, ins in instructions.items():
            if not isinstance(ins, BranchInstruction):
                continue
            target_address = address + (ins.offset + 1) * 8
            logging.debug(f"{type(ins).__name__} at 0x{ins.address:x} "
                          f"with target 0x{target_address}")
            ins.set_target(instructions[target_address])
        return instructions.values()
