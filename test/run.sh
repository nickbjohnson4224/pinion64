#!/bin/sh

echo " QEMU	images/pinion.iso"
tools/bin/qemu-system-x86_64 -cdrom images/pinion.iso -serial stdio -display none
