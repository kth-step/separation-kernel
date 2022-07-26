#!/usr/bin/env bash
#
# Kill QEMU_SYSTEM 
function cleanup {
        kill $(jobs -p)
}
trap cleanup SIGINT SIGTERM EXIT

$QEMU_SYSTEM -M virt -smp 5 -m 2G \
        -kernel $ELF -nographic    \
        -bios none -s -S

if $(test -z $!)
then
        echo "Failed to launch QEMU."
        exit 10
fi

# x-terminal-emulator -e                       \
# $GDB $ELF -ex "target remote localhost:1234" \
#           -ex "b _start"                     \
#           -ex "b __hang"                     \
#           -ex "b __mhang"                     \
#           -ex "c"                            \
#           -ex "layout split" \
#           -ex "fs cmd"
