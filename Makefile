# See LICENSE file for copyright and license details.
PROGRAM ?=separation-kernel
BUILD   ?=build

TARGET=$(ELF) $(BIN)
ELF=$(BUILD)/$(PROGRAM).elf
BIN=$(BUILD)/$(PROGRAM).bin

LDS        ?=config.lds
CONFIG_H   ?=config.h
PLATFORM_H ?=bsp/virt.h

SRCS=$(wildcard src/*.[cS])
OBJS=$(patsubst %, $(BUILD)/%.o, $(SRCS))
DEPS=$(patsubst %, $(BUILD)/%.d, $(SRCS))
GEN_HDRS=inc/gen/cap.h inc/gen/asm_consts.h 
HDRS=$(wildcard inc/*.h) $(GEN_HDRS) $(CONFIG_H) $(PLATFORM_H)
DA=$(patsubst %, %.da, $(TARGET))


# Tools
PREFIX ?=riscv64-unknown-elf
CC=$(PREFIX)-gcc
SIZE=$(PREFIX)-size
OBJCOPY=$(PREFIX)-objcopy
OBJDUMP=$(PREFIX)-objdump

ARCH   ?=rv64imac
ABI    ?=lp64
CMODEL ?=medany

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+=-fPIC -fno-pie
CFLAGS+=-Iinc
CFLAGS+=-gdwarf-2
CFLAGS+=-O2
CFLAGS+=-include $(PLATFORM_H) -include $(CONFIG_H) 

.PHONY: all target clean size da cloc format
.SECONDARY:

all: target

target: $(TARGET)

inc/gen/cap.h: gen/cap.yml scripts/cap_gen.py 
	@echo -e "GEN\t$@"
	@mkdir -p $(@D) 
	@./scripts/cap_gen.py $< > $@

inc/gen/asm_consts.h: gen/asm_consts.c inc/proc.h inc/cap_node.h inc/consts.h
	@echo -e "GEN\t$@"
	@mkdir -p $(@D) 
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

$(BUILD)/%.S.o: %.S inc/gen/asm_consts.h $(CONFIG_H) $(PLATFORM_H)
	@echo -e "CC\t$@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(BUILD)/%.c.o: %.c inc/gen/cap.h $(CONFIG_H) $(PLATFORM_H)
	@echo -e "CC\t$@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

%.elf: $(OBJS) $(LDS)
	@echo -e "CC\t$@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -o $@ $(SRCS)

%.bin: %.elf
	@echo -e "OBJCOPY\t$@"
	@$(OBJCOPY) -O binary $< $@

%.da: %.elf
	@echo -e "OBJDUMP\t$@"
	@$(OBJDUMP) -d $< > $@

clean:
	@echo "Cleaning separation-kernel"
	@rm -f $(OBJS) $(DEPS) $(GEN_HDRS) $(TARGET) $(DA)

size:
	@$(SIZE) $(OBJS) $(TARGET)

cloc:
	@cloc $(HDRS) $(SRCS)

format:
	clang-format -i $(filter inc/%.h, $(HDRS)) $(filter src/%.c, $(SRCS))

-include $(DEPS)
