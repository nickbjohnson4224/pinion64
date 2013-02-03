#!/bin/sh

echo " QEMU	images/pinion.iso"
qemu-system-x86_64 -cdrom images/pinion.iso -serial stdio -smp 8,threads=2 -display none
