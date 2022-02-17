# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build

include config.mk

ASM_SRCS=$(wildcard *.S)
C_SRCS=$(filter-out offsets.c,$(wildcard *.c))

OBJS=$(addprefix $(BUILD_DIR)/, $(ASM_SRCS:.S=.o) $(C_SRCS:.c=.o))
DEPS=$(addprefix $(BUILD_DIR)/, $(ASM_SRCS:.S=.d) $(C_SRCS:.c=.d))
ELF=$(BUILD_DIR)/$(PROGRAM).elf
DA=$(BUILD_DIR)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-Iinclude
CFLAGS+=-std=gnu18
CFLAGS+=-Og -g

ASFLAGS=-march=$(ARCH) -mabi=$(ABI)
ASFLAGS+=-Iinclude
ASFLAGS+=-g

LDFLAGS=-nostdlib

# Commands

.PHONY: all settings clean size
all: settings $(ELF) $(DA)

settings:
	@echo build options:
	@echo "CC  	= $(CC)"
	@echo "LD    	= $(LD)"
	@echo "CFLAGS	= $(CFLAGS)"
	@echo "ASFLAGS	= $(ASFLAGS)"
	@echo "LDFLAGS	= $(LDFLAGS)"

clean:
	rm -f $(OBJS) $(DEPS) $(ELF) include/offsets.h

size:
	$(SIZE) $(ELF)

debug-qemu: $(ELF)
	GDB=$(GDB) QEMU_SYSTEM=$(QEMU_SYSTEM) ELF=$(ELF) \
	scripts/debug-qemu.sh

# Build instructions

$(BUILD_DIR):
	mkdir -p $@

$(OBJS) $(ELF): | $(BUILD_DIR) include/offsets.h

include/offsets.h: offsets.c include/types.h
	CC=$(CC) scripts/gen-offsets.sh

$(BUILD_DIR)/%.o: %.S
	$(CC) $(ASFLAGS) -MD -c -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -MD -c -o $@ $<

$(BUILD_DIR)/%.elf: $(OBJS)
	$(LD) $(LDFLAGS) -T $(LDS) -o $@ $(OBJS)

$(BUILD_DIR)/%.da: $(ELF)
	$(OBJDUMP) -d $< > $@

-include $(DEPS)
