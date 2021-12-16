#!/bin/bash

$QEMU_SYSTEM -machine virt -kernel $ELF \
      -nographic -bios none -s -S &
QEMU_PID=$!

if [[ -z $QEMU_PID ]]; then
        echo "Failed to launch QEMU."
        exit 10
fi

$GDB $ELF -ex "target remote localhost:1234" \
          -ex "b _start" \
          -ex "c" \
          -ex "layout split"

kill -s SIGTERM $QEMU_PID
