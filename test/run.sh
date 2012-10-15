#!/bin/sh

echo " QEMU	images/pinion.iso"
qemu-system-x86_64 -cdrom images/pinion.iso -display none -serial stdio -d int,cpu_reset
