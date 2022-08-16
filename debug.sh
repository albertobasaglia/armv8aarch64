#!/bin/bash

make
qemu-system-aarch64 -machine virt,highmem=on,gic-version=3 -cpu cortex-a53 -smp 1 \
        -m 2G -kernel kernel/kernel.elf -s -S -nographic -d int,trace:*gic*

# aarch64-none-elf-gdb
