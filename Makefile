# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build

include config.mk

S_SRCS=$(wildcard src/*.S) 
C_SRCS=$(wildcard src/*.c)
C_HDRS=$(wildcard src/*.h)

AES_PATH=crypto-app/wolfssl/
AES_OBJ=$(AES_PATH)aes.o
AES_LIB=$(AES_PATH)lib_aes.a

S_SRCS+=$(wildcard crypto-app/*.S)
C_SRCS+=$(wildcard crypto-app/*.c)
C_HDRS+=$(wildcard crypto-app/*.h)
C_SRCS:=$(filter-out crypto-app/test.c, $(C_SRCS))

ELF=$(BUILD_DIR)/$(PROGRAM).elf
DA=$(BUILD_DIR)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-O2 -gdwarf-2
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+= -DDEBUG
CFLAGS+=-Wall -fanalyzer

# Commands

.PHONY: all settings format clean size debug-qemu qemu noninteractive-qemu debug-hifive-unleashed stop-qemu stop-debugging stop-debugging-qemu benchmark-qemu benchmark-board
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

noninteractive-qemu: $(ELF)
	@QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/noninteractive-qemu.sh

--debug-hifive-unleashed:
	@sed -i 's/#define QEMU_DEBUGGING 1/#define QEMU_DEBUGGING 0/' src/config.h

debug-hifive-unleashed: --debug-hifive-unleashed $(ELF)
	@scripts/hifive-unleashed.sh
	@sed -i 's/#define QEMU_DEBUGGING 0/#define QEMU_DEBUGGING 1/' src/config.h

stop-qemu:
	@killall qemu-system-riscv64

stop-debugging:
	@killall riscv64-unknown-elf-gdb

stop-debugging-qemu: stop-qemu stop-debugging

benchmark-qemu: $(ELF)
	@sed -i 's/is_qemu = False/is_qemu = True/' scripts/benchmark-script.py
	@python3 scripts/benchmark-script.py

benchmark-board:
	@sed -i 's/is_qemu = True/is_qemu = False/' scripts/benchmark-script.py
	@python3 scripts/benchmark-script.py

# Build instructions

$(AES_OBJ): $(AES_PATH)aes.c
	@$(CC) $(CFLAGS) -o $@ $(AES_PATH)aes.c -c

$(AES_LIB): $(AES_OBJ)
	ar -vq $(AES_LIB) $(AES_OBJ)

$(BUILD_DIR):
	@mkdir -p $@

$(ELF): Makefile config.mk | $(BUILD_DIR)

$(BUILD_DIR)/%.elf: $(C_HDRS) $(C_SRCS) $(S_SRCS) config.lds $(AES_LIB)
	@echo "Compiling ELF file:\t$(S_SRCS) $(C_SRCS) ==> $@ \n"
	@$(CC) $(CFLAGS) -o $@ $(S_SRCS) $(C_SRCS) $(AES_LIB)

$(BUILD_DIR)/%.da: $(ELF)
	@echo "Producing DA file:\t$^ ==> $@"
	@$(OBJDUMP) -d $< > $@
