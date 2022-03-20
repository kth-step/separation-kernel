# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build

include config.mk

SRCS=$(wildcard src/*.S) $(wildcard src/*.c)
HDRS=$(wildcard src/*.h)

ELF=$(BUILD_DIR)/$(PROGRAM).elf
DA=$(BUILD_DIR)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-O2 -g
CFLAGS+= -T$(LDS) -nostartfiles

# Commands

.PHONY: all settings clean size
all: settings $(ELF) $(DA)

settings:
	@echo "Build options:"
	@echo "  CC      = $(CC)"
	@echo "  OBJDUMP = $(OBJDUMP)"
	@echo "  CFLAGS  = $(CFLAGS)"
	@echo ""

clean:
	@echo "Cleaning"
	@rm -f $(ELF) $(DA) inc/offsets.h

size:
	@echo "Size of binary:"
	@$(SIZE) $(ELF)

debug-qemu: $(ELF)
	@GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/debug-qemu.sh

# Build instructions

$(BUILD_DIR):
	@mkdir -p $@

$(ELF): Makefile config.mk | $(BUILD_DIR)

$(BUILD_DIR)/%.elf: $(HDRS) $(SRCS) config.lds
	@echo "Linking ELF file:\t$(SRCS) ==> $@"
	@$(CC) $(CFLAGS) -o $@ $(SRCS)

$(BUILD_DIR)/%.da: $(ELF)
	@echo "Producing DA file:\t$^ ==> $@"
	@$(OBJDUMP) -d $< > $@
