# S3K - Simply Secure Separation Kernel

Simply Secure Separation Kernel (S3K) is a real-time separation kernel for embedded RISC-V applications. S3K aims to spatially and temporally isolate all partitions running on the RISC-V processor. S3K relies on RISC-V's PMP mechanism to isolate the partitions, no virtual memory is required.

## User guide

Install the GNU RISC-V toolchain:
```
git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git
cd riscv-gnu-toolchain
git checkout 051b9f7ddb7d136777505ea19c70a41926842b96 # Optional
./configure --prefix=/opt/riscv --enable-multilib --with-cmodel=medany
sudo make install -j <nr of CPUs>
```

Install QEMU:
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

