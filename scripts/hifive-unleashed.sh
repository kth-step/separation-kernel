#!/bin/bash -e

function cleanup {
        echo "Killing all processes"
        kill $(jobs -p)
}
trap cleanup SIGINT SIGTERM EXIT

echo "!!! Loading program"
riscv64-unknown-elf-gdb \
        -ex "target remote localhost:3333" \
        -ex 'load build/s3k.elf' \
        -ex 'detach' \
        -ex 'quit' >/dev/null

echo "!!! Program loaded"

echo "!!! Connecting to GDB sessions"
for port in 3337 3336 3335 3334; do
        x-terminal-emulator -e \
                riscv64-unknown-elf-gdb \
                -ex "target remote localhost:$port" \
                -ex 'symbol-file build/s3k.elf' \
                -ex 'set $mtvec=0x80000000' \
                -ex 'set $pc=0x80000000' &
        sleep 0.25
done
echo "!!! All GDB sessions started"
echo "!!! Kill this process to kill all GDB sessions"

wait
