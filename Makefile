# See LICENSE file for copyright and license details.
PROGRAM=s3k
LDS=config.lds
BUILD_DIR=build

include config.mk

ASM_SRCS=boot.S entry.S user.S
C_SRCS=

OBJS=$(addprefix $(BUILD_DIR)/,$(ASM_SRCS:.S=.o) $(C_SRCS:.c=.o))
ELF=$(BUILD_DIR)/$(PROGRAM).elf
DA=$(BUILD_DIR)/$(PROGRAM).da

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+=-Og -g
ASFLAGS=-march=$(ARCH) -mabi=$(ABI)
ASFLAGS+=-g
LDFLAGS=-nostdlib --no-relax 

# Commands

.PHONY: all clean size run-qemu run-gdb
all: $(ELF) $(DA)

clean:
	rm -f $(OBJS) $(ELF) offsets.h

size:
	$(SIZE) $(ELF)

debug-qemu: $(ELF)
	GDB=$(GDB) \
	QEMU_SYSTEM=$(QEMU_SYSTEM) \
	ELF=$(ELF) \
	scripts/qemu-debug.sh

# Build instructions

$(BUILD_DIR):
	mkdir -p $@

$(OBJS) $(ELF): | $(BUILD_DIR) offsets.h

offsets.h: offsets.c
	CC=$(CC) \
	scripts/gen-offsets.sh

$(BUILD_DIR)/%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS)  -c -o $@ $<

$(BUILD_DIR)/%.elf: $(OBJS)
	$(LD) $(LDFLAGS) -T $(LDS) -o $@ $(OBJS)

$(BUILD_DIR)/%.da: $(ELF)
	$(OBJDUMP) -d $< > $@
