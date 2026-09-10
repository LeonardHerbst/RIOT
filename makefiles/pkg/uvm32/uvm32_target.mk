UVM32_INCLUDES += -I$(RIOTBASE)/pkg/uvm32/include/shared
UVM32_INCLUDES += -I$(RIOTBASE)/pkg/uvm32/include/target

UVM32_OBJ ?= $(UVM32_SRC:.c=.o)
UVM32_ELF ?= $(UVM32_SRC:.c=.elf)
UVM32_BIN ?= $(UVM32_SRC:.c=.bin)

UVM32_INIT_ASM ?= ${RIOTBASE}/makefiles/pkg/uvm32/crt0.S

PREFIX       := riscv64-unknown-elf-
OPT          ?= -Os
override UVM32_CFLAGS += ${OPT} -fno-stack-protector -fno-builtin-memcpy -fno-builtin
override UVM32_CFLAGS += -static-libgcc -fdata-sections -ffunction-sections
override UVM32_CFLAGS += -g -march=rv32im -mabi=ilp32 -static
LDFLAGS      := -T ${RIOTBASE}/makefiles/pkg/uvm32/linker.ld -nostdlib -Wl,--gc-sections
LIBS         := -lgcc # needed for softfp

$(UVM32_BIN): $(UVM32_SRC)
	${PREFIX}gcc -o $(UVM32_ELF) ${UVM32_CFLAGS} $(UVM32_INCLUDES) ${LDFLAGS} ${UVM32_SRC} $(UVM32_INIT_ASM) ${LIBS}
	@$(PREFIX)objcopy $(UVM32_ELF) -O binary ${UVM32_BIN}

dump: $(UVM32_BINARY)
	$(PREFIX)objdump -S -d -f $(UVM32_ELF)

clean:
	rm -f ${UVM32_OBJ} $(UVM32_ELF) $(UVM32_ELF)

