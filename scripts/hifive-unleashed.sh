#!/bin/bash -e

function cleanup {
        echo "Killing all processes"
        kill $(jobs -p)
}
trap cleanup SIGINT SIGTERM EXIT

echo "!!! Loading program"

# 1. Connect to target.
# 2. Reset the system and halt all cores.
# 3. Continue core 0, initializing DDR and more. Once breakpoint is reached, next step.
# 4. Load the elf file to the DDR memory.
# 5. Detach from the remote target and quit.
riscv64-unknown-elf-gdb \
        --batch \
        -iex 'set pagination off' \
        -iex "target remote localhost:3333" \
        -iex 'mon reset halt' \
        -iex 'continue' \
        -iex 'load build/s3k.elf' \
        -iex 'detach' \
        -iex 'quit'


echo "!!! Program loaded"

echo "!!! Connecting to GDB sessions"
for port in 3337 3336 3335 3334; do
        x-terminal-emulator -e \
                riscv64-unknown-elf-gdb build/s3k.elf \
                -q \
                -iex 'set pagination off' \
                -iex "target remote localhost:$port" \
                -iex 'symbol-file build/s3k.elf' \
                -iex 'set $pc=0x80000000' \
                -iex 'continue' &
        sleep 0.25
done
echo "!!! All GDB sessions started"
echo "!!! Kill this process to kill all GDB sessions"

wait
