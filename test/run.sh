#!/bin/sh

echo " QEMU	images/pinion.iso"
tools/bin/qemu-system-x86_64 -cdrom images/pinion.iso -display none -serial stdio -smp 8,threads=2
