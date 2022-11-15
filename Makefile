# See LICENSE file for copyright and license details.
.POSIX:

PROGRAM ?= separation-kernel
BUILD ?= build

TARGET=$(ELF) $(BIN) $(DA)
ELF=$(BUILD)/$(PROGRAM).elf
BIN=$(BUILD)/$(PROGRAM).bin
DA=$(BUILD)/$(PROGRAM).da

LDS        ?=config.lds
S3K_CONFIG_H   ?=config.h
PLATFORM_H ?=bsp/virt.h

OBJS=$(patsubst %.c, build/%.o, $(wildcard src/*.c)) $(patsubst %.S, build/%.o, $(wildcard src/*.S))

GEN_HDRS=src/inc/cap.h src/inc/asm_consts.h
HDRS=$(wildcard src/inc/*.h) $(GEN_HDRS) $(CONFIG_H) $(PLATFORM_H)

API_GEN_HDRS=api/s3k_cap.h api/s3k_consts.h

# Tools
RISCV_PREFIX ?=riscv64-unknown-elf
CC 		=$(RISCV_PREFIX)-gcc
SIZE 	=$(RISCV_PREFIX)-size
OBJCOPY =$(RISCV_PREFIX)-objcopy
OBJDUMP =$(RISCV_PREFIX)-objdump

ARCH   ?=rv64imac
ABI    ?=lp64
CMODEL ?=medany

CFLAGS+=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+= -T$(LDS) -nostartfiles -nostdlib -ffreestanding -static
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+=-gdwarf-2
CFLAGS+=-O2
CFLAGS+=-Isrc/inc -include $(PLATFORM_H) -include $(S3K_CONFIG_H) 

ifneq "$(PAYLOAD)" ""
CFLAGS+=-DPAYLOAD=\"$(PAYLOAD)\"
endif

.PHONY: all target clean size da cloc format api elf bin da
.SECONDARY:

all: $(TARGET)

elf: $(ELF)
bin: $(BIN)
da: $(DA)

api/s3k.h: api
api: api/s3k_cap.h api/s3k_consts.h

clean:
	rm -f $(ELF) $(BIN) $(DA) $(OBJS) $(GEN_HDRS) $(API_GEN_HDRS)

size:
	$(SIZE) $(OBJS) $(TARGET)

cloc:
	cloc $(HDRS) $(SRCS)

format:
	clang-format -i $(filter src/inc/%.h, $(HDRS)) $(filter src/%.c, $(SRCS)) $(wildcard api/*.h)

$(BUILD):
	mkdir -p $(BUILD)

src/inc/cap.h: gen/cap.yml
	./scripts/gen_cap $< $@

src/inc/asm_consts.h: gen/asm_consts.c $(CAP_H) src/inc/proc.h src/inc/cap_node.h src/inc/consts.h
	CC=$(CC) CFLAGS="$(CFLAGS)" ./scripts/gen_asm_consts $< $@

$(BUILD)/%.o: %.S $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.c $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ELF): $(OBJS) $(LDS) $(PAYLOAD)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(DA): $(ELF)
	$(OBJDUMP) -D $< > $@

api/s3k_cap.h: src/inc/cap.h
	sed '/kassert/d' $< > $@

api/s3k_consts.h: src/inc/consts.h
	cp $< $@
