# See LICENSE file for copyright and license details.
LDS		=config.lds
TARGET	=$(BUILD)/s3k.elf
BUILD  ?=.

include config.mk

SRC=$(wildcard src/*.[cS]) $(wildcard bsp/$(BSP)/*.[cS])
OBJ=$(patsubst %, $(BUILD)/%.o, $(SRC))
DEP=$(patsubst %, $(BUILD)/%.d, $(SRC))
GEN_HDR=inc/offsets.g.h inc/cap.g.h
HDR=$(wildcard inc/*.h) $(GEN_HDR)

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
.PHONY: all target debug release

all: debug 

debug:
	$(MAKE) BUILD=debug target

release:
	$(MAKE) BUILD=release target

target: $(TARGET)

inc/cap.g.h: gen/cap.yml scripts/cap_gen.py 
	@echo "Generating $@"
	@./scripts/cap_gen.py $< > $@

inc/offsets.g.h: gen/offsets.c inc/proc.h inc/cap_node.h inc/cap.g.h
	@echo "Generating $@"
	@$(CC) $(CFLAGS) -S -o - $< | grep -oE "#\w+ .*" > $@

%.o: $(GEN_HDR)

$(BUILD)/%.c.o: %.c
	@mkdir -p $(@D) 
	@echo "Compiling C object $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(BUILD)/%.S.o: %.S
	@mkdir -p $(@D) 
	@echo "Compiling ASM object $@"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

$(TARGET): $(OBJ) $(LDS)
	@echo "Linking ELF $@"
	@$(CC) $(CFLAGS) -o $@ $(OBJ)

-include $(DEP)
