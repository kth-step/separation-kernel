# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD=build

include config.mk

SRCS=$(wildcard src/*.[cS]) $(wildcard bsp/$(BSP)/*.[cS])
OBJS=$(patsubst %, $(BUILD)/%.o, $(SRCS))
HDRS=$(wildcard src/*.h)
DEPS=$(patsubst %.o, %.d, $(OBJS))

ELF=$(BUILD)/$(PROGRAM).elf
DA=$(BUILD)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-O2
CFLAGS+=-gdwarf-2
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Ibsp/$(BSP)
CFLAGS+=-Wall -fanalyzer 
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
	@rm -f $(ELF) $(DA) $(DEPS) $(OBJS) src/cap_utils.h

size: $(ELF)
	@echo "Size of binary:"
	@$(SIZE) $(OBJS) $(ELF)

cloc:
	@cloc $(wildcard src/*.c src/*.h src/*.S)

qemu: $(ELF)
	@GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) scripts/debug-qemu.sh

src/cap.h: scripts/cap_gen.py cap.yml
	@echo "Generating $@"
	@./scripts/cap_gen.py cap.yml > $@

$(BUILD)/%.c.o: %.c src/cap.h
	@mkdir -p $(@D) 
	@echo "Compiling $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.S.o: %.S
	@mkdir -p $(@D) 
	@echo "Compiling $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(ELF): $(OBJS) $(LDS)
	@echo "Compiling $@"
	@$(CC) $(CFLAGS) -o $@ $(OBJS)

%.da: %.elf
	@echo "Disassembling $<"
	@$(OBJDUMP) -d $< > $@


-include $(DEPS)
