# See LICENSE file for copyright and license details.
.POSIX:

PROGRAM ?=separation-kernel
BUILD   ?=build

TARGET=$(ELF) $(BIN)
ELF=$(BUILD)/$(PROGRAM).elf
BIN=$(BUILD)/$(PROGRAM).bin

LDS        ?=config.lds
CONFIG_H   ?=config.h
PLATFORM_H ?=bsp/virt.h

C_SRCS=$(wildcard src/*.c)
S_SRCS=$(wildcard src/*.S)
OBJS=$(patsubst %.c, $(BUILD)/%.o, $(C_SRCS)) \
     $(patsubst %.S, $(BUILD)/%.o, $(S_SRCS))
DEPS=$(patsubst %.c, $(BUILD)/%.d, $(C_SRCS)) \
     $(patsubst %.S, $(BUILD)/%.d, $(S_SRCS))
CAP_H=inc/gen/cap.h
ASM_CONST_H=inc/gen/asm_const.h
HDRS=$(wildcard inc/*.h) $(CAP_H) $(ASM_CONST_H) $(CONFIG_H) $(PLATFORM_H)
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
CFLAGS+=-include $(PLATFORM_H) -include $(CONFIG_H) 
CFLAGS+=-Iinc

.PHONY: all target clean size da cloc format api
.SECONDARY:

all: target

target: $(TARGET) 

$(CAP_H): gen/cap.yml scripts/cap_gen.py 
	@printf "  GEN\t$@\n"
	@mkdir -p $(@D) 
	@./scripts/cap_gen.py $< > $@

$(ASM_CONST_H): gen/asm_consts.c inc/proc.h inc/cap_node.h inc/consts.h
	@printf "  GEN\t$@\n"
	@mkdir -p $(@D) 
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

$(BUILD)/%.o: %.S $(ASM_CONST_H) $(CONFIG_H) $(PLATFORM_H) $(PAYLOAD)
	@printf "  CC\t$@\n"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(BUILD)/%.o: %.c $(CAP_H) $(CONFIG_H) $(PLATFORM_H)
	@printf "  CC\t$@\n"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(ELF): $(OBJS) $(LDS) 
	@printf "  CC\t$@\n"
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -o $@ $(OBJS)

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
	@rm -f $(OBJS) $(DEPS) $(CAP_H) $(ASM_CONST_H) $(TARGET) $(DA)

size:
	@$(SIZE) $(OBJS) $(TARGET)

cloc:
	@cloc $(HDRS) $(SRCS)

format:
	clang-format -i $(filter inc/%.h, $(HDRS)) $(filter src/%.c, $(SRCS)) $(wildcard api/*.h)

-include $(DEPS)
