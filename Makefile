# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD=build

include config.mk

SRCS=$(filter-out src/offsets.c, $(wildcard src/*.[cS])) $(wildcard bsp/$(BSP)/*.[cS])
OBJS=$(patsubst %, $(BUILD)/%.o, $(SRCS))
HDRS=$(wildcard src/*.h) src/cap.h src/offsets.h
DEPS=$(patsubst %.o, %.d, $(OBJS))

ELF=$(BUILD)/$(PROGRAM).elf
DA=$(BUILD)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-O3
CFLAGS+=-gdwarf-2
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Ibsp/$(BSP)
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+= -DNDEBUG

# Commands
.PHONY: all settings format clean size cloc qemu tags

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
	@rm -f $(ELF) $(DA) $(DEPS) $(OBJS) src/cap.h src/s3k_cap.h src/offsets.h

size: $(ELF)
	@echo "Size of binary:"
	@$(SIZE) $(OBJS) $(ELF)

cloc:
	@cloc $(wildcard src/*.c src/*.h src/*.S)

qemu: $(ELF) $(DA)
	@GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/debug-qemu.sh

tags:
	@ctags $(SRCS) $(HDRS)

src/cap.h: scripts/cap_gen.py cap.yml
	@echo "Generating $@"
	@./scripts/cap_gen.py cap.yml > $@

src/s3k_cap.h: src/cap.h
	@echo "Generating $@"
	@sed '/kassert/d' $< > $@

src/offsets.h: src/offsets.c src/proc.h src/cap_node.h src/cap.h
	@echo "Generating $@"
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

$(BUILD)/%.c.o: %.c src/cap.h src/s3k_cap.h src/offsets.h
	@mkdir -p $(@D) 
	@echo "Compiling C object $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(BUILD)/%.S.o: %.S src/offsets.h
	@mkdir -p $(@D) 
	@echo "Compiling ASM object $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(ELF): $(OBJS) $(LDS)
	@echo "Linking ELF $@"
	@$(CC) $(CFLAGS) -o $@ $(OBJS)

%.da: %.elf
	@echo "Disassembling ELF $<"
	@$(OBJDUMP) -d $< > $@


-include $(DEPS)
