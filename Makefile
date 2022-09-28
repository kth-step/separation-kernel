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
RISCV_PREFIX ?=riscv64-unknown-elf
CC=$(RISCV_PREFIX)-gcc
SIZE=$(RISCV_PREFIX)-size
OBJCOPY=$(RISCV_PREFIX)-objcopy
OBJDUMP=$(RISCV_PREFIX)-objdump

ARCH   ?=rv64imac
ABI    ?=lp64
CMODEL ?=medany

CFLAGS =-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+= -T$(LDS) -nostartfiles
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+=-gdwarf-2
CFLAGS+=-Og
ifneq "$(PAYLOAD)" ""
CFLAGS+=-DPAYLOAD=\"$(PAYLOAD)\"
endif
CFLAGS+=-include $(PLATFORM_H) -include $(CONFIG_H) 
CFLAGS+=-Iinc

.PHONY: all target clean size da cloc format api
.SECONDARY:

all: target

target: $(TARGET) 

inc/gen/cap.h: gen/cap.yml scripts/cap_gen.py 
	@printf "  GEN\t$@\n"
	@mkdir -p $(@D) 
	@./scripts/cap_gen.py $< > $@

inc/gen/asm_consts.h: gen/asm_consts.c inc/proc.h inc/cap_node.h inc/consts.h
	@printf "  GEN\t$@\n"
	@mkdir -p $(@D) 
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

$(BUILD)/src/payload.S.o: src/payload.S $(PAYLOAD)
$(BUILD)/%.S.o: %.S inc/gen/asm_consts.h $(CONFIG_H) $(PLATFORM_H)
	@printf "  CC\t$@\n"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<



$(BUILD)/%.c.o: %.c inc/gen/cap.h $(CONFIG_H) $(PLATFORM_H)
	@printf "  CC\t$@\n"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(ELF): $(OBJS) $(LDS) 
	@printf "  CC\t$@\n"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -o $@ $(SRCS)

$(BIN): $(ELF)
	@printf "  OBJCOPY\t$@\n"
	@$(OBJCOPY) -O binary $< $@

$(DA): $(ELF)
	@printf "  OBJDUMP\t$@\n"
	@$(OBJDUMP) -d $< > $@

api/s3k_cap.h: inc/gen/cap.h
	@printf "  GEN\t$@\n"
	@sed '/kassert/d' inc/gen/cap.h > api/s3k_cap.h

api/s3k_consts.h: inc/const.h
	@printf "  GEN\t$@\n"
	@cp inc/consts.h api/s3k_consts.h

api: api/s3k_consts.h api/s3k_cap.h

clean:
	@echo "  CLEANING"
	@rm -f $(OBJS) $(DEPS) $(GEN_HDRS) $(TARGET) $(DA)

size:
	@$(SIZE) $(OBJS) $(TARGET)

cloc:
	@cloc $(HDRS) $(SRCS)

format:
	clang-format -i $(filter inc/%.h, $(HDRS)) $(filter src/%.c, $(SRCS)) $(wildcard api/*.h)

-include $(DEPS)
