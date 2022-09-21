# See LICENSE file for copyright and license details.
LDS		=config.lds
TARGET	=$(BUILD)/s3k.elf

include config.mk

BUILD  ?=debug
SRC=$(wildcard src/*.[cS])
OBJ=$(patsubst %, $(BUILD)/%.o, $(SRC))
DEP=$(patsubst %, $(BUILD)/%.d, $(SRC))
GEN_HDR=inc/asm_consts.g.h inc/cap.g.h
HDR=$(wildcard inc/*.h) $(GEN_HDR)
DA=$(patsubst %.elf, %.da, $(TARGET))

CFLAGS+=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-Iinc
CFLAGS+=-std=gnu18
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Ibsp/$(BSP)
CFLAGS+=-Wall -fanalyzer -Werror

ifeq "$(BUILD)" "debug"
CFLAGS+=-gdwarf-2
CFLAGS+=-Og
else
CFLAGS+=-O2
CFLAGS+= -DNDEBUG
endif

# Commands
.PHONY: all settings target debug release clean

all: target 

settings:
	@echo "Compile settings:"
	@echo " TARGET=$(TARGET)"
	@echo "     CC=$(CC)"
	@echo " CFLAGS=$(CFLAGS)"
	@echo "    SRC=$(SRC)"
	@echo

target: settings $(TARGET) $(DA)

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

$(TARGET): $(OBJ) $(LDS)
	@echo "Linking ELF $@"
	@$(CC) $(CFLAGS) -o $@ $(OBJ)

$(DA): $(TARGET)
	@echo "Disassembling $@"
	@$(OBJDUMP) -d $< > $@

-include $(DEP)

# Extra targets
.PHONY: cloc format size

# Count Lines Of Code
cloc: $(GEN_HDR)
	@echo "Counting lines of code"
	@cloc $(HDR) $(GEN_HDR) $(SRC)

# Calculating size of binary
size: 
	@echo "Calculating size of binaries"
	@$(SIZE) $(TARGET) $(OBJ)

format:
	@echo "Formatting source code"
	@clang-format -i $(HDR) $(filter %.s, $(SRC))
