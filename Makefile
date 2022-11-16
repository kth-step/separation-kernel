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
BSP        ?=bsp/virt

C_SRCS=$(wildcard $(SRC)/*.c)
S_SRCS=$(wildcard $(SRC)/*.S)
OBJS=$(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(C_SRCS)) \
     $(patsubst $(SRC)/%.S, $(BUILD)/%.o, $(S_SRCS))
DEPS=$(OBJS:.o=.d)
HDRS=$(wildcard $(INC)/*.h) $(CAP_H) $(ASM_CONST_H) $(CONFIG_H) $(PLATFORM_H)
DA=$(OBJS:.p=.da) $(ELF:.elf=.da)

CAP_H=$(INC)/cap.h
ASM_CONSTS_H=$(INC)/asm_consts.h

# Tools
RISCV_PREFIX ?=riscv64-unknown-elf
CC=$(RISCV_PREFIX)-gcc
#CC=ccomp
LD=$(RISCV_PREFIX)-ld
SIZE=$(RISCV_PREFIX)-size
OBJCOPY=$(RISCV_PREFIX)-objcopy
OBJDUMP=$(RISCV_PREFIX)-objdump

ARCH   ?=rv64imac
ABI    ?=lp64
CMODEL ?=medany

CFLAGS =-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-Wall
CFLAGS+=-gdwarf-2 -O0
CFLAGS+=-MMD
CFLAGS+=-nostdlib
CFLAGS+=-I$(INC) -I$(BSP) -include $(BSP).h -include $(CONFIG_H) 

CFLAGS+=-T$(LDS) -ffreestanding -no-pie

.PHONY: all target clean size da cloc format elf bin da api
.SECONDARY:

all: $(ELF)

elf: $(ELF)
bin: $(BIN)
da: $(DA)


clean:
	rm -f $(OBJS) $(DEPS) $(CAP_H) $(ASM_CONST_H) $(ELF) $(BIN) $(DA)

size:
	$(SIZE) $(OBJS) $(ELF)

cloc:
	cloc $(HDRS) $(SRCS)

format:
	clang-format -i $(wildcard inc/*.h) $(wildcard src/*.c) $(wildcard api/*.h)

$(BUILD):
	mkdir -p $(BUILD)

# Generated headers
$(CAP_H): $(GEN)/cap.yml
	$(SCRIPTS)/gen_cap $< $@

$(ASM_CONSTS_H): $(GEN)/asm_consts.c $(INC)/proc.h $(INC)/cap_node.h $(INC)/consts.h
	$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@ 

# Payload
ifneq ("$(PAYLOAD)","")
$(BUILD)/payload.o: $(SRC)/payload.S $(PAYLOAD) | $(BUILD)
	$(CC) $(CFLAGS) -DPAYLOAD=\"$(PAYLOAD)\" -MMD -c -o $@ $<
endif

# Kernel
$(BUILD)/%.o: $(SRC)/%.S $(ASM_CONSTS_H) $(CONFIG_H) $(PLATFORM_H) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: $(SRC)/%.c $(CAP_H) $(CONFIG_H) $(PLATFORM_H) | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ELF): $(OBJS) $(LDS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(BUILD)/%.da: $(BUILD)/%.o
	$(OBJDUMP) -d $< > $@

$(BUILD)/%.da: $(BUILD)/%.elf
	$(OBJDUMP) -d $< > $@

# API
api: $(API)/s3k_consts.h $(API)/s3k_cap.h

$(API)/s3k_cap.h: $(INC)/cap.h
	cp $< $@
	sed -i '/kassert/d' $@

$(API)/s3k_consts.h: $(INC)/consts.h
	cp $< $@

-include $(DEPS)
