# S3K - Simply Secure Separation Kernel

Simply Secure Separation Kernel (S3K) is a real-time separation kernel for embedded RISC-V applications. S3K aims to spatially and temporally isolate all partitions running on the RISC-V processor. S3K relies on RISC-V's PMP mechanism to isolate the partitions, no virtual memory is required.

## Basic API

### System calls (Basic)

- `void s3k_get_pid()` - Get the process's PID.
- `uint64_t s3k_read_reg(register_number)` - Read a virtual register.
- `void s3k_write_reg(register_number, value)` - Write to a virtual register.
- `void s3k_yield()` - Yield the remainder of the time slice (TODO: rename the function?).
- `cap_t s3k_read_cap(i)` - Read capability from slot `i`.
- `uint64_t s3k_move_cap(i, j)` - Move a capability in slot `i` to slot `j`.
- `uint64_t s3k_delete_cap(i)` -  Delete capability in slot `i`.
- `uint64_t s3k_revoke_cap(i) - Delete all children of capability in slot `i`.
- `uint64_t s3k_derive_cap(i, j, cap) - Derive capability `cap` from capability in slot `i` and place in slot `j`.

### System calls (Capability invocation)
The following system calls are pseudo system calls implemented on `s3k_invoke_cap(i, a1, a2, a3, a4, a5, a6, a7)`.

**Supervisor Invocations,** the `i` of the following system calls should point at a supervisor capability.
- `uint64_t s3k_supervisor_suspend(i, pid)` - Suspend process `pid`.
- `uint64_t s3k_supervisor_resume(i, pid)` - Resume process `pid`.
- `uint64_t s3k_supervisor_get_state(i, pid)` - Get state of process `pid`.
- `uint64_t s3k_supervisor_read_reg(i, pid, register_number)` - Reads a virtual register of process `pid`. (Req. process `pid` suspended).
- `uint64_t s3k_supervisor_write_reg(i, pid, register_number, value)` - Write to virtual register of process `pid`. (Req. process `pid` suspended).
- `uint64_t s3k_supervisor_read_cap(i, pid, j)` - Read capability `j` of process `pid`. (Req. process `pid` suspended).
- `uint64_t s3k_supervisor_give_cap(i, pid, j, k)` - Give capability `j` to process `pid`, placing it in slot `k`. (Req. process `pid` suspended).
- `uint64_t s3k_supervisor_take_cap(i, pid, j, k)` - Take capability `j` from process `pid`, placing it in slot `k`. (Req. process `pid` suspended).

### Virtual registers
TODO: Fix constants for virtual registers.

## User guide

### Install the RISC-V GCC compiler
Install the GNU RISC-V toolchain (around 16 GB of space required):
```
# Install the prerequisites, see https://github.com/riscv-collab/riscv-gnu-toolchain

# Clone repository
git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git
cd riscv-gnu-toolchain

# --with-cmodel=medany: This is required to get stdlib working on non-low memory.
# --prefix=<path>: Install location.
# --enable-multilib: Allows us to compile to many versions of RISC-V. Could be replaced with something more specific.
./configure --prefix=/opt/riscv --enable-multilib --with-cmodel=medany

# Make the toolchain, installs automatically to /opt/riscv, no need for 'make install'
# Do not compile with "make linux", this would create a toolchain for Linux on RISC-V,
# not embedded software on RISC-V.
sudo make -j <nr of CPUs>

# Need to add path to tools, I add this in bashrc.
export PATH=/opt/riscv:$PATH
```
*You can also install the toolchain from SiFive.*

### 
```
# Clone this repository:
git clone git@github.com:kth-step/separation-kernel.git
cd separation-kernel

# Other prerequisites, python3 for generating files.
make
make debug-qemu # Runs the kernel using QEMU
```

## Coding style

- Functions variables should use `snake_case`.
- Function names should be prefixed with the file name, e.g., `proc_init()` if the function resides in `file.c`
- Structs, enum, and union must be defined with `snake_case` and have a typedef in the form `snake_case_t`.
- C files should be formatted using `clang-format` (`make format`), style file (`.clang-format`) is provided.

## Progress

- [x] Implement the foundation of the scheduler.
- [x] Implement message passing so we can send basic messages with no capabilities.
- [x] Implement message passing so we can send time slices, allowing us to dynamically change the scheduling.
- [x] Implement the supervisor capability allowing process 0 to manage other process's capabilities.
- [x] Implement memory protection (memory slice, memory frame).
- [x] Implement exception handling.
- [ ] Separate kernel and user binaries.  Implement a basic bootloader in process 0 for loading the user binaries.
- [ ] Design and build applications, benchmarks and tests.
- [ ] Start on formal verification.

