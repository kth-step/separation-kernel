# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build

include config.mk

SRCS=$(wildcard *.S) $(filter-out offsets.c,$(wildcard *.c)) 
HDRS=$(wildcard inc/*.h) inc/offsets.h

ELF=$(BUILD_DIR)/$(PROGRAM).elf
DA=$(BUILD_DIR)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-Og -g
CFLAGS+= -T$(LDS) -static -nostartfiles
# Treat registers t5 and t6 as saved registers
#CFLAGS+= -fcall-saved-t5 -fcall-saved-t6
CFLAGS+= -ffixed-t5 -ffixed-t6

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

inc/offsets.h: offsets.c $(filter-out inc/offsets.h, $(HDRS)) scripts/gen-offsets.sh
	@echo "Generating offsets:\t$< ==> $@"
	@CC=$(CC) scripts/gen-offsets.sh $< $@

$(BUILD_DIR)/%.elf: $(HDRS) $(SRCS) config.lds
	@echo "Linking ELF file:\t$(SRCS) ==> $@"
	@$(CC) $(CFLAGS) -o $@ $(SRCS)

$(BUILD_DIR)/%.da: $(ELF)
	@echo "Producing DA file:\t$^ ==> $@"
	@$(OBJDUMP) -d $< > $@
