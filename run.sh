#!/bin/bash

make -C kernel/

qemu-system-aarch64 -machine virt,highmem=on,gic-version=3 \
                    -cpu cortex-a53 \
                    -smp 1 \
                    -m 2G \
                    -kernel kernel/kernel.elf \
                    -global virtio-mmio.force-legacy=false \
                    -device virtio-blk-device,drive=hd \
                    -drive file=hdd,if=none,format=raw,id=hd \
                    -nographic \
                    -d int &
                    # -d int,trace:virtio* &
sleep 5
kill -9 $!
