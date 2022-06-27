# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build
OBJ_DIR=obj

include config.mk

OBJS=$(patsubst src/%.c, $(OBJ_DIR)/%.c.o, $(wildcard src/*.c)) \
     $(patsubst src/%.S, $(OBJ_DIR)/%.S.o, $(wildcard src/*.S))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(OBJ_DIR)/%.d, $(OBJS))
DA=$(patsubst %.o, %.da, $(OBJS))


ELF=$(BUILD_DIR)/$(PROGRAM).elf
BIN=$(BUILD_DIR)/$(PROGRAM).bin
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
all: settings $(ELF) $(DA) $(BIN)

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
$(BUILD_DIR) $(OBJ_DIR):
	@mkdir -p $@

inc/cap_utils.h: scripts/cap_gen.py data/cap.yml
	./scripts/cap_gen.py data/cap.yml > inc/cap_utils.h

$(OBJ_DIR)/%.c.o: src/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.S.o: src/%.S | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ELF): $(OBJS) $(LDS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(OBJ_DIR)/%.da: $(OBJ_DIR)/%.o
	$(OBJDUMP) -d $< > $@

$(BUILD_DIR)/%.da: $(BUILD_DIR)/%.elf
	$(OBJDUMP) -d $< > $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	$(OBJCOPY) -S -O binary $^ $@

-include $(DEPS)
