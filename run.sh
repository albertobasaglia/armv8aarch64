#!/bin/bash

make -C kernel/
qemu-system-aarch64 -machine virt,highmem=on,gic-version=3 -cpu cortex-a53 -smp 1 \
        -m 2G -kernel kernel/kernel.elf -nographic
