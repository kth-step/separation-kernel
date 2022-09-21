.PHONY: all target

CC=riscv64-unknown-elf-gcc
TARGET=payload.bin

all: target

target: $(TARGET)

$(TARGET): dummy.S
	$(CC) -o $@ $< -T dummy.lds -nostartfiles
