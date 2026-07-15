EMBPF_SOURCES ?= $(wildcard $(CURDIR)/*.c)
EMBPF_GENEMBPF ?= $(RIOTBASE)/dist/tools/rbpf/gen_rbf.py

EMBPF_BINS = $(EMBPF_SOURCES:.c=.bin)
EMBPF_OBJS = $(EMBPF_SOURCES:.c=.o)


LLC ?= llc
CLANG ?= clang
INC_FLAGS = -nostdinc -isystem `$(CLANG) -print-file-name=include`
EXTRA_CFLAGS ?= -Os
EMBPFINCLUDE =  -I$(RIOTBASE)/drivers/include \
								-I$(RIOTBASE)/core/include \
								-I$(RIOTBASE)/core/lib/include \
								-I$(RIOTBASE)/sys/include \
								-I$(RIOTBASE)/build/pkg/femto-container/include

all: $(EMBPF_BINS)

.PHONY: clean

clean:
	rm -f $(EMBPF_OBJS)

INC_FLAGS = -nostdinc -isystem `$(CLANG) -print-file-name=include`

$(EMBPF_OBJS):  %.o:%.c
	$(CLANG) $(INC_FLAGS) \
	        $(EMBPFINCLUDE) \
	        -Wno-unused-value -Wno-pointer-sign -g3\
	        -Wno-compare-distinct-pointer-types \
	        -Wno-gnu-variable-sized-type-not-at-end \
	        -Wno-address-of-packed-member -Wno-tautological-compare \
	        -Wno-unknown-warning-option \
	        --target=bpf -mcpu=v2 -Xclang -target-feature -Xclang +alu32 \
	        $(EXTRA_CFLAGS) -c $< -o $@

$(EMBPF_BINS): %.bin:%.o
	$(EMBPF_GENEMBPF) -d generate $(EMBPF_GENEMBPF_FLAGS) $< $@
