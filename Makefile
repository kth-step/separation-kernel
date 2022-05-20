# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build

include config.mk

S_SRCS=$(wildcard src/*.S) 
C_SRCS=$(wildcard src/*.c)
C_HDRS=$(wildcard src/*.h)

ELF=$(BUILD_DIR)/$(PROGRAM).elf
DA=$(BUILD_DIR)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-O2 -gdwarf-2
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+= -DDEBUG
CFLAGS+=-Wall -fanalyzer

# Commands

.PHONY: all settings format clean size debug-qemu qemu
all: settings $(ELF) $(DA)

settings:
	@echo "Build options:"
	@echo "  CC      = $(CC)"
	@echo "  OBJDUMP = $(OBJDUMP)"
	@echo "  CFLAGS  = $(CFLAGS)"
	@echo

format:
	@echo "Formatting code"
	@clang-format -i $(H_HDRS) $(C_SRCS) 

clean:
	@echo "Cleaning"
	@rm -f $(ELF) $(DA)

size:
	@echo "Size of binary:"
	@$(SIZE) $(ELF)

qemu debug-qemu: $(ELF)
	@GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/debug-qemu.sh

# Build instructions

$(BUILD_DIR):
	@mkdir -p $@

$(ELF): Makefile config.mk | $(BUILD_DIR)

$(BUILD_DIR)/%.elf: $(C_HDRS) $(C_SRCS) $(S_SRCS) config.lds
	@echo "Compiling ELF file:\t$(S_SRCS)$(C_SRCS) ==> $@"
	@$(CC) $(CFLAGS) -o $@ $(S_SRCS) $(C_SRCS)

$(BUILD_DIR)/%.da: $(ELF)
	@echo "Producing DA file:\t$^ ==> $@"
	@$(OBJDUMP) -d $< > $@
