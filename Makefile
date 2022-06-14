# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build

include config.mk

OBJS=$(patsubst src/%.c, $(BUILD_DIR)/%.c.o, $(wildcard src/*.c)) \
     $(patsubst src/%.S, $(BUILD_DIR)/%.S.o, $(wildcard src/*.S))
DEPS=$(patsubst %.o, %.d, $(OBJS))
DA=$(patsubst %.o, %.da, $(OBJS))


ELF=$(BUILD_DIR)/$(PROGRAM).elf
DA+=$(BUILD_DIR)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-O2
CFLAGS+=-gdwarf-2
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+=-Iinc
CFLAGS+=-MMD
#CFLAGS+= -DNDEBUG

# Commands
.PHONY: all settings format clean size cloc qemu

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
	@rm -f $(ELF) $(DA) $(DEPS) $(OBJS)

size: $(ELF)
	@echo "Size of binary:"
	@$(SIZE) $(OBJS) $(ELF)

cloc:
	@cloc $(wildcard src/*.c src/*/*.c inc/*.h src/*.S)

qemu: $(ELF)
	@GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/debug-qemu.sh

# Make target directory
$(BUILD_DIR) obj:
	@mkdir -p $@

$(BUILD_DIR)/%.c.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.S.o: src/%.S | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ELF): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.da: $(BUILD_DIR)/%.o
	$(OBJDUMP) -d $< > $@

$(BUILD_DIR)/%.da: $(BUILD_DIR)/%.elf
	$(OBJDUMP) -d $< > $@

-include $(DEPS)
