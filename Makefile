# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD=build

include config.mk

SRCS=$(wildcard src/*.[cS]) $(wildcard bsp/$(BSP)/*.[cS])
OBJS=$(patsubst %, $(BUILD)/%.o, $(SRCS))
HDRS=$(wildcard inc/*.h)
DEPS=$(patsubst %.o, %.d, $(OBJS))
GEN_HDRS=inc/offsets.g.h inc/cap.g.h

ELF=$(BUILD)/$(PROGRAM).elf
DA=$(BUILD)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-Iinc
CFLAGS+=-std=gnu18
CFLAGS+=-Og
CFLAGS+=-gdwarf-2
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Ibsp/$(BSP)
CFLAGS+=-Wall -fanalyzer -Werror
#CFLAGS+= -DNDEBUG

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
	@clang-format -i $(wildcard src/*.[ch]) $(wildcard bsp/*/*.[ch])

clean:
	@echo "Cleaning"
	@rm -f $(ELF) $(DA) $(DEPS) $(OBJS) $(GEN_HDRS)

size: $(ELF)
	@echo "Size of binary:"
	@$(SIZE) $(OBJS) $(ELF)

cloc:
	@cloc $(SRCS) $(HDRS) $(GEN_HDRS)

qemu: $(ELF) $(DA)
	@GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/debug-qemu.sh

tags:
	@ctags $(SRCS) $(HDRS)

inc/cap.g.h: gen/cap.yml scripts/cap_gen.py 
	@echo "Generating $@"
	@./scripts/cap_gen.py $< > $@

inc/offsets.g.h: gen/offsets.c inc/proc.h inc/cap_node.h inc/cap.g.h
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
