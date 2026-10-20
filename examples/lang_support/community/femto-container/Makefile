# application name
APPLICATION = femto_example

# default BOARD environment
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../../../

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

ifeq ($(PROGRAM), )
$(error Specify a program to run!)
endif

USEMODULE += ztimer_usec
USEMODULE += ztimer_msec
USEPKG += femto-container
USEPKG += mpaland-printf

PROGRAM_FILE = $(CURDIR)/programs/$(PROGRAM).c
PROGRAM_INCLUDES = $(CURDIR)/programs
ITERATIONS ?= 1

BLOBS += program.bin

RIOT_STACK ?= 9192
STACK ?= 1024
CALL_DEPTH ?= 8 # default eBPF call depth.

CFLAGS += -DTHREAD_STACKSIZE_MAIN=$(RIOT_STACK)
CFLAGS += -DFEMTO_CONTAINER_STACK_SIZE=$(STACK)
CFLAGS += -DFEMTO_CONTAINER_CALL_DEPTH=$(CALL_DEPTH)
CFLAGS += -DFEMTO_CONTAINER_ENABLE_ALU32
CFLAGS += -DFEMTO_CONTAINER_BRANCHES_ALLOWED=2000000
CFLAGS += -DFEMTO_CONTAINER_USER_HELPER
CFLAGS += -DDEBUG_ASSERT_VERBOSE
CFLAGS += -DFEMTO_CONTAINER_PRETTY_ERROR

all: $(BLOBS)

.PHONY: program.bin clean-ebpf
program.bin: $(PROGRAM_FILE)
	$(MAKE) -C $(CURDIR)/programs all EMBPF_SOURCES=$(PROGRAM_FILE) EMBPF_INCLUDES=$(PROGRAM_INCLUDES)
	cp $(CURDIR)/programs/$(PROGRAM).o $(CURDIR)/program.o
	cp $(CURDIR)/programs/$(PROGRAM).bin $(CURDIR)/program.bin
	rm $(CURDIR)/programs/*.bin || true
	rm $(CURDIR)/programs/*.o || true

dump:
	$(MAKE) -C $(CURDIR)/programs dump DUMP_FILE=$(CURDIR)/program

clean: clean-ebpf

include $(RIOTBASE)/Makefile.include
