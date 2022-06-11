#!/bin/bash -e

$QEMU_SYSTEM -M virt -smp 2, -m 8G \
             -kernel $ELF \
             -nographic                 \
             -bios none -s &

if $(test -z $!)
then
        echo "Failed to launch QEMU."
        exit 10
fi