# S3K - Simply Secure Separation Kernel

Simply Secure Separation Kernel (S3K) is a real-time separation kernel for embedded RISC-V applications. S3K aims to spatially and temporally isolate all partitions running on the RISC-V processor. S3K relies on RISC-V's PMP mechanism to isolate the partitions, no virtual memory is required.

## Coding style

- Large functions should use CamelCase, small functions should use snake_case.
- Non-visible functions (static functions to a `.c` file) should have two underlines as a prefix, e.g., `__hidden_function`.
- Variables should use snake_case.
- Structs, enum, and union must be defined with snake_case and have a typedef in CamelCase.
- Indentation and so on, use clang-format with `make format` and check the `.clang-format` file
- Functions and types of the same file must have a common prefix. For examples, `cap_delete`, `CapDelete`, `CapType`, `ProcState`, `ProcInitProcesses`, `AsmTrapEntry`, `AsmTrapExit`, and more.
- Globally visible assembly functions should start with the prefix `Asm`.

## User guide

Install the GNU RISC-V toolchain (around 16 GB of space required):
```
git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git
cd riscv-gnu-toolchain
# Install prerequisites (Debian)
sudo apt-get install autoconf automake autotools-dev curl python3 libmpc-dev \
        libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf \
        libtool patchutils bc zlib1g-dev libexpat-dev

# Optional, checkout working branch
git checkout 051b9f7ddb7d136777505ea19c70a41926842b96 

# --with-cmodel=medany: This is required to get stdlib working on non-low memory.
# --prefix=<path>: Install location.
# --enable-multilib: Allows us to compile to many versions of RISC-V. Could be replaced with something more specific.
./configure --prefix=/opt/riscv --enable-multilib --with-cmodel=medany

# Make the toolchain, installs automatically to /opt/riscv
# Do not compile with "make linux", this would create a toolchain for Linux on RISC-V,
# not embedded software on RISC-V.
sudo make -j <nr of CPUs>

# Need to add path to tools, I add this in bashrc.
export PATH=/opt/riscv:$PATH
```
You can also install the toolchain from SiFive.

Install QEMU if you want to run the program in QEMU:
```
sudo apt install qemu-system-misc # Debian, maybe also Ubuntu
```


Clone this repository:
```
git clone git@github.com:kth-step/separation-kernel.git
cd separation-kernel
make
make debug-qemu # Runs the kernel using QEMU
```

## Progress

- [x] Implement the foundation of the scheduler. Done, currently schedules process 0 only, but can be configured statically to behave differently. DONE!
- [ ] Implement message passing so we can send basic messages with no capabilities.
- [ ] Implement message passing so we can send time slices, allowing us to dynamically change the scheduling.
- [ ] Separate kernel and user binaries.  Implement a basic bootloader in process 0 for loading the user binaries.
- [ ] Implement the supervisor capability allowing process 0 to manage other process's capabilities.
- [ ] Implement memory protection (memory slice, memory frame).
- [ ] Implement exception handling.
- [ ] Design and build applications, benchmarks and tests.
- [ ] Start on formal verification.

