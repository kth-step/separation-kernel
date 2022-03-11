#!/bin/bash -e

# Kill QEMU_SYSTEM 
function cleanup {
        kill $(jobs -p)
}
trap cleanup SIGINT SIGTERM EXIT

$QEMU_SYSTEM -M virt -smp 2, -m 8G \
             -kernel $ELF \
             -nographic                 \
             -bios none -s -S &

if $(test -z $!)
then
        echo "Failed to launch QEMU."
        exit 10
fi

x-terminal-emulator -e                       \
$GDB $ELF -ex "target remote localhost:1234" \
          -ex "b _start"                     \
          -ex "c"                            \
          -ex "layout split"
