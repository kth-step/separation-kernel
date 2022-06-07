# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build

include config.mk

SRCS=$(wildcard src/*.S) $(wildcard src/*.c)
HDRS=$(wildcard inc/*.h)
DEPS=$(HDRS) $(SRCS) $(wildcard src/*/*.c) $(LDS)

ELF=$(BUILD_DIR)/$(PROGRAM).elf
DA=$(BUILD_DIR)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-O2 -gdwarf-2
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+= -DDEBUG
CFLAGS+=-Wall -fanalyzer
CFLAGS+=-Iinc
CFLAGS+=-MMD

# Commands
.PHONY: all settings format clean size qemu

# Show settings, compile elf and make a disassembly by default.
all: settings $(ELF) $(DA)

settings:
	@echo "Build options:"
	@echo "  CC      = $(CC)"
	@echo "  OBJDUMP = $(OBJDUMP)"
	@echo "  CFLAGS  = $(CFLAGS)"
	@echo

format:
	@echo "Formatting code"
	@clang-format -i $(HDRS) $(filter %.c, $(SRCS)) 

clean:
	@echo "Cleaning"
	@rm -f $(ELF) $(DA)

size: $(ELF)
	@echo "Size of binary:"
	@$(SIZE) $(ELF)

qemu: $(ELF)
	@GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/debug-qemu.sh

# Make target directory
$(BUILD_DIR):
	@mkdir -p $@

# If makefile or config changes, rebuild elf.
$(ELF): Makefile config.mk | $(BUILD_DIR)

# Compile elf file
$(ELF): $(DEPS)
	@echo "Compiling ELF file: $(SRCS) ==> $@"
	@$(CC) $(CFLAGS) -o $@ $(SRCS)

# Disassmble elf file
$(DA): $(ELF)
	@echo "Producing DA file: $^ ==> $@"
	@$(OBJDUMP) -d $< > $@
