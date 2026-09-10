# name of your application
APPLICATION = uvm32-example

PROGRAM ?= hello_world

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../..

UVM32_MK ?= $(RIOTBASE)/makefiles/pkg/uvm32/uvm32_target.mk

USEPKG += uvm32
USEMODULE += uvm32_default_syscalls

CFLAGS += -DUVM32_MEMORY_SIZE=9192 -DUVM32_ERROR_STRINGS -DUVM32_DEFAULT_SYSCALLS

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

BLOBS += program.bin

UVM32_SRC ?= $(CURDIR)/program/$(PROGRAM).c

all: $(BLOBS)
	@echo Building the binary for $(PROGRAM)

program.bin: $(UVM32_SRC)
	make -C $(CURDIR)/program -f $(UVM32_MK) UVM32_SRC=$(UVM32_SRC) UVM32_CFLAGS="-DUVM32_DEFAULT_SYSCALLS"
	cp $(CURDIR)/program/$(PROGRAM).bin $(CURDIR)/program.bin
	rm $(CURDIR)/program/$(PROGRAM).bin

dump: program.bin
	make -C $(CURDIR)/program -f $(UVM32_MK) UVM32_SRC=$(UVM32_SRC) UVM32_CFLAGS="-DUVM32_DEFAULT_SYSCALLS" dump
	rm $(CURDIR)/program/$(PROGRAM).bin

clean-binary:
	rm program.bin
	make -C $(CURDIR)/program -f $(UVM32_MK) UVM32_SRC=$(UVM32_SRC) UVM32_CFLAGS="-DUVM32_DEFAULT_SYSCALLS" clean


include $(RIOTBASE)/Makefile.include
