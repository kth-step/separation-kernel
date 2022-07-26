#!/usr/bin/env bash

$GDB $ELF -ex "target remote localhost:1234" \
          -ex "b __hang"                     \
          -ex "b __mhang"                     \
          -ex "c" \
          -ex "kill" \
          -ex "quit"
