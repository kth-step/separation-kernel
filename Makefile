# See LICENSE file for copyright and license details.
.POSIX:

PROGRAM ?=separation-kernel

BUILD=build
SRC=src
INC=inc
GEN=gen
SCRIPTS=scripts

ELF=$(BUILD)/$(PROGRAM).elf
BIN=$(BUILD)/$(PROGRAM).bin
API=api

LDS        ?=config.lds
CONFIG_H   ?=config.h
PLATFORM_H ?=bsp/virt.h

OBJS=$(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(wildcard $(SRC)/*.c)) \
	$(patsubst $(SRC)/%.S, $(BUILD)/%.o, $(wildcard $(SRC)/*.S))
DEPS=$(OBJS:.o=.d)
HDRS=$(wildcard $(INC)/*.h) $(CAP_H) $(ASM_CONST_H) $(CONFIG_H) $(PLATFORM_H)
DA=$(patsubst %.elf, %.da, $(ELF))

CAP_H=$(INC)/cap.h
ASM_CONST_H=$(INC)/asm_const.h

# Tools
RISCV_PREFIX ?=riscv64-unknown-elf
CC=$(RISCV_PREFIX)-gcc
LD=$(RISCV_PREFIX)-ld
SIZE=$(RISCV_PREFIX)-size
OBJCOPY=$(RISCV_PREFIX)-objcopy
OBJDUMP=$(RISCV_PREFIX)-objdump

ARCH   ?=rv64imac
ABI    ?=lp64
CMODEL ?=medany

CFLAGS+=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+=-gdwarf-2
CFLAGS+=-Og
CFLAGS+=-MMD
CFLAGS+=-c
CFLAGS+=-I$(INC) -include $(PLATFORM_H) -include $(CONFIG_H) 

LDFLAGS+=-static -no-pie --relax -nostdlib
LDFLAGS+=-T$(LDS)

.PHONY: all target clean size da cloc format elf bin da api
.SECONDARY:

all: $(ELF)

elf: $(ELF)
bin: $(BIN)
da: $(DA)


clean:
	rm -f $(OBJS) $(DEPS) $(CAP_H) $(ASM_CONST_H) $(ELF) $(BIN) $(DA)

size:
	$(SIZE) $(OBJS) $(TARGET)

cloc:
	cloc $(HDRS) $(SRCS)

format:
	clang-format -i $(filter src/inc/%.h, $(HDRS)) $(filter src/%.c, $(SRCS)) $(wildcard api/*.h)

$(BUILD):
	mkdir -p $(BUILD)

# Generated headers
$(CAP_H): $(GEN)/cap.yml
	$(SCRIPTS)/gen_cap $< $@

$(ASM_CONSTS_H): $(GEN)/asm_consts.c $(INC)/proc.h $(INC)/cap_node.h $(INC)/consts.h
	CC=$(CC) CFLAGS="$(CFLAGS)" $(SCRIPTS)/gen_asm_consts $< $@

# Payload
ifneq ("$(PAYLOAD)","")
$(BUILD)/payload.o: $(SRC)/payload.S $(PAYLOAD) | $(BUILD)
	$(CC) $(CFLAGS) -DPAYLOAD=\"$(PAYLOAD)\" -MMD -c -o $@ $<
endif

# Kernel
$(BUILD)/%.o: $(SRC)/%.S $(INC)/asm_consts.h $(CONFIG_H) $(PLATFORM_H) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)/%.o: $(SRC)/%.c $(CAP_H) $(CONFIG_H) $(PLATFORM_H) | $(BUILD)
	$(CC) $(CFLAGS) -o $@ $<

$(ELF): $(OBJS) $(LDS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(DA): $(ELF)
	$(OBJDUMP) -d $< > $@

# API
api: $(API)/s3k_consts.h $(API)/s3k_cap.h

$(API)/s3k_cap.h: $(INC)/cap.h
	cp $< $@
	sed -i '/kassert/d' $@

$(API)/s3k_consts.h: $(INC)/consts.h
	cp $< $@
