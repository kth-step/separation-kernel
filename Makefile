# See LICENSE file for copyright and license details.
LDS		 =config.lds
ELF	     =$(BUILD)/s3k.elf
BIN	     =$(BUILD)/s3k.bin
TARGET   =$(ELF) $(BIN)
BSP	     ?=virt
CONFIG_H ?=./config.h
BUILD    ?=debug

include config.mk

SRC=$(wildcard src/*.[cS])
OBJ=$(patsubst %, $(BUILD)/%.o, $(SRC))
DEP=$(patsubst %, $(BUILD)/%.d, $(SRC))
GEN_HDR=inc/asm_consts.g.h inc/cap.g.h
HDR=$(wildcard inc/*.h) $(GEN_HDR) $(CONFIG_H)
DA=$(patsubst %.elf, %.da, $(ELF))

CFLAGS+=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-Iinc -I$(dir $(CONFIG_H))
CFLAGS+=-std=gnu18
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Ibsp/$(BSP)
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+=-fPIC -fno-pie

ifeq "$(BUILD)" "debug"
CFLAGS+=-gdwarf-2
CFLAGS+=-Og
else
CFLAGS+=-O2
CFLAGS+= -DNDEBUG
SRC:=$(filter-out src/info.c src/snprintf.c src/kprint.c, $(SRC))
endif

# Commands
.PHONY: all settings target clean

all: target 

target: $(TARGET)
elf: $(ELF)
bin: $(BIN)

clean:
	@echo "Cleaning"
	@git clean -xfd

inc/cap.g.h: gen/cap.yml scripts/cap_gen.py 
	@echo "Generating $@"
	@./scripts/cap_gen.py $< > $@

inc/asm_consts.g.h: gen/asm_consts.c inc/proc.h inc/cap_node.h inc/cap.g.h inc/consts.h
	@echo "Generating $@"
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

$(BUILD)/%.c.o: %.c $(GEN_HDR)
	@mkdir -p $(@D) 
	@echo "Compiling C object $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(BUILD)/%.S.o: %.S $(GEN_HDR)
	@mkdir -p $(@D) 
	@echo "Compiling ASM object $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(ELF): $(OBJ) $(LDS)
	@echo "Linking ELF $@"
	@$(CC) $(CFLAGS) -o $@ $(OBJ)

$(BIN): $(ELF)
	@echo "Converting $@"
	@$(OBJCOPY) -O binary $< $@


-include $(DEP)

# Extra targets
.PHONY: cloc format size

disassemble: $(DA)

$(DA): $(ELF)
	@echo "Disassembling $@"
	@$(OBJDUMP) -D $< > $@

# Count Lines Of Code
cloc: $(GEN_HDR)
	@echo "Counting lines of code"
	@cloc $(HDR) $(GEN_HDR) $(SRC)

# Calculating size of binary
size: $(TARGET) $(OBJ)
	@echo "Calculating size of binaries"
	@$(SIZE) $(OBJ) $(TARGET) 

format:
	@echo "Formatting source code"
	@clang-format -i $(HDR) $(filter %.s, $(SRC))
