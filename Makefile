# See LICENSE file for copyright and license details.
.POSIX:

PROGRAM ?= separation-kernel
BUILD ?= build

TARGET=$(ELF) $(BIN) $(DA)
ELF=$(BUILD)/$(PROGRAM).elf
BIN=$(BUILD)/$(PROGRAM).bin
DA=$(BUILD)/$(PROGRAM).da

LDS        ?=config.lds
CONFIG_H   ?=config.h
PLATFORM_H ?=bsp/virt.h

OBJS=$(patsubst %.c, build/%.o, $(wildcard src/*.c)) $(patsubst %.S, build/%.o, $(wildcard src/*.S))

GEN_HDRS=inc/cap.h inc/asm_consts.h
HDRS=$(wildcard inc/*.h) $(GEN_HDRS) $(CONFIG_H) $(PLATFORM_H)

API_GEN_HDRS=api/s3k_cap.h api/s3k_consts.h

# Tools
RISCV_PREFIX ?=riscv64-unknown-elf
CC=$(RISCV_PREFIX)-gcc
SIZE=$(RISCV_PREFIX)-size
OBJCOPY=$(RISCV_PREFIX)-objcopy
OBJDUMP=$(RISCV_PREFIX)-objdump

ARCH   ?=rv64imac
ABI    ?=lp64
CMODEL ?=medany

CFLAGS+=-march=$(ARCH) -mabi=$(ABI) -mcmodel=$(CMODEL)
CFLAGS+=-std=gnu18
CFLAGS+= -T$(LDS) -nostartfiles -nostdlib -ffreestanding -static
CFLAGS+=-Wall -fanalyzer -Werror
CFLAGS+=-gdwarf-2
CFLAGS+=-O2
CFLAGS+=-Iinc -include $(PLATFORM_H) -include $(CONFIG_H) 

ifneq "$(PAYLOAD)" ""
CFLAGS+=-DPAYLOAD=\"$(PAYLOAD)\"
endif

.PHONY: all target clean size da cloc format api
.SECONDARY:

all: $(TARGET)

api: $(API_GEN_HDRS)

clean:
	@printf "CLEAN\t$(PROGRAM)\n"
	@rm -f $(OBJS) $(ELF) $(BIN) $(DA) $(GEN_HDRS) $(API_GEN_HDRS)

size:
	@$(SIZE) $(OBJS) $(TARGET)

cloc:
	cloc $(HDRS) $(SRCS)

format:
	clang-format -i $(filter inc/%.h, $(HDRS)) $(filter src/%.c, $(SRCS)) $(wildcard api/*.h)


$(BUILD):
	@mkdir -p $(BUILD)

inc/cap.h: gen/cap.yml scripts/cap_gen.py
	@printf "GEN\t$@\n"
	@./scripts/cap_gen.py $< > $@

inc/asm_consts.h: gen/asm_consts.c $(CAP_H) inc/proc.h inc/cap_node.h inc/consts.h
	@printf "GEN\t$@\n"
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

$(BUILD)/%.o: %.S $(HDRS) inc/asm_consts.h | $(BUILD)
	@printf "CC\t$@\n"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.c $(HDRS) inc/cap.h | $(BUILD)
	@printf "CC\t$@\n"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(ELF): $(OBJS) $(LDS) $(PAYLOAD)
	@printf "CC\t$@\n"
	@$(CC) $(CFLAGS) -o $@ $(OBJS)

$(BIN): $(ELF)
	@printf "OBJCOPY\t$@\n"
	@$(OBJCOPY) -O binary $< $@

$(DA): $(ELF)
	@printf "OBJDUMP\t$@\n"
	@$(OBJDUMP) -D $< > $@

api/s3k_cap.h: inc/cap.h
	sed '/kassert/d' inc/cap.h > api/s3k_cap.h

api/s3k_consts.h: inc/consts.h
	cp inc/consts.h api/s3k_consts.h
