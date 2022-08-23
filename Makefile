# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD=build

include config.mk

SRCS=$(filter-out src/offsets.c, $(wildcard src/*.[cS])) $(wildcard bsp/$(BSP)/*.[cS])
OBJS=$(patsubst %, $(BUILD)/%.o, $(SRCS))
HDRS=$(filter-out src/%.g.h, $(wildcard src/*.h))
DEPS=$(patsubst %.o, %.d, $(OBJS))
GEN_HDRS=src/cap.g.h src/s3k_cap.g.h src/offsets.g.h

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

format:
	@echo "Formatting code"
	@clang-format -i $(HDRS) $(filter %.c, $(SRCS)) 

clean:
	@echo "Cleaning"
	@rm -f $(ELF) $(DA) $(DEPS) $(OBJS) src/*.g.h

size: $(ELF)
	@echo "Size of binary:"
	@$(SIZE) $(OBJS) $(ELF)

cloc:
	@cloc $(SRCS) $(HDRS) scripts/cap_gen.py cap.yml

qemu: $(ELF) $(DA)
	@GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/debug-qemu.sh

tags:
	@ctags $(SRCS) $(HDRS)

src/cap.g.h: scripts/cap_gen.py cap.yml
	@echo "Generating $@"
	@./scripts/cap_gen.py cap.yml > $@

src/s3k_cap.g.h: src/cap.g.h
	@echo "Generating $@"
	@sed '/kassert/d' $< > $@

src/offsets.g.h: src/offsets.c src/proc.h src/cap_node.h src/cap.g.h
	@echo "Generating $@"
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

$(BUILD)/%.c.o: %.c $(GEN_HDRS)
	@mkdir -p $(@D) 
	@echo "Compiling C object $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(BUILD)/%.S.o: %.S $(GEN_HDRS)
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
