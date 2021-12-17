# See LICENSE file for copyright and license details.

# Prefix of GNU toolchain
RISCV_PREFIX=riscv64-unknown-elf-

# Set tool variables
AR=$(RISCV_PREFIX)ar
AS=$(RISCV_PREFIX)as
CC=$(RISCV_PREFIX)gcc
CXX=$(RISCV_PREFIX)g++
CPP=$(RISCV_PREFIX)cpp
ELFEDIT=$(RISCV_PREFIX)elfedit
COV=$(RISCV_PREFIX)gcov
GDB=$(RISCV_PREFIX)gdb
PROF=$(RISCV_PREFIX)gprof
LD=$(RISCV_PREFIX)ld
NM=$(RISCV_PREFIX)nm
OBJCOPY=$(RISCV_PREFIX)objcopy
OBJDUMP=$(RISCV_PREFIX)objdump
READELF=$(RISCV_PREFIX)readelf
SIZE=$(RISCV_PREFIX)size
STRING=$(RISCV_PREFIX)string
STRIP=$(RISCV_PREFIX)strip

# Qemu tool
QEMU_SYSTEM=qemu-system-riscv64
QEMU=qemu-riscv64

# Set architecture, ABI and code model (march, mabi, mcmodel)
ARCH=rv64imac
ABI=lp64
CMODEL=medlow
