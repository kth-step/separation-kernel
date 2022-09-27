# See LICENSE file for copyright and license details.
TARGET ?=s3k.elf
LDS    ?=config.lds
CONFIG ?=config.h
PLATFORM ?=bsp/virt.h
BUILD  ?=build

SRCS+=$(wildcard src/*.[cS])
OBJS=$(patsubst %, $(BUILD)/%.o, $(SRCS))
DEPS=$(patsubst %, $(BUILD)/%.d, $(SRCS))
GEN_HDRS=inc/gen/cap.h inc/gen/asm_consts.h 
HDRS=$(wildcard inc/*.h) $(GEN_HDRS) $(CONFIG) $(PLATFORM)
DA=$(patsubst %.elf, %.da, $(ELF))

ARCH?=rv64imac
ABI?=lp64
CMODEL?=medany

CFLAGS=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+=-fPIC -fno-pie
CFLAGS+=-Iinc
CFLAGS+=-gdwarf-2
CFLAGS+=-O2

CFLAGS+=-include $(PLATFORM) -include $(CONFIG) 

.PHONY: all clean
.SECONDARY:

all: $(TARGET)

inc/gen/cap.h: gen/cap.yml scripts/cap_gen.py 
	@echo -e "GEN\t$@"
	@mkdir -p $(@D) 
	@./scripts/cap_gen.py $< > $@

inc/gen/asm_consts.h: gen/asm_consts.c inc/proc.h inc/cap_node.h inc/consts.h
	@echo -e "GEN\t$@"
	@mkdir -p $(@D) 
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

$(BUILD)/%.S.o: %.S inc/gen/asm_consts.h
	@echo -e "CC\t$@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(BUILD)/%.c.o: %.c inc/gen/cap.h
	@echo -e "CC\t$@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

%.elf: $(OBJS) $(LDS)
	@echo -e "CC\t$@"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -o $@ $(SRCS)

%.bin: $(BUILD)/%.elf
	@echo -e "OBJCOPY\t$@"
	@$(OBJCOPY) -O binary $< $@

clean:
	rm -f $(OBJS) $(DEPS) $(GEN_HDRS)

-include $(DEPS)
