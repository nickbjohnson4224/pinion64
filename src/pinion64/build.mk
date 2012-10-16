# Build parameters for module 'pinion64'

CFLAGS_pinion64  := -std=c99 -MMD
CFLAGS_pinion64  += -pedantic -Wall -Wextra -Wno-unused-parameter -Werror
CFLAGS_pinion64  += -m64 -fPIC -mno-red-zone -fvisibility=hidden -static
CFLAGS_pinion64  += -mno-sse -mno-mmx -mno-sse2 -mno-sse3 -mno-3dnow
CFLAGS_pinion64  += -ffreestanding
CFLAGS_pinion64  += -O3 -fomit-frame-pointer
ASFLAGS_pinion64 := -felf64
LDFLAGS_pinion64 := -melf_x86_64 -z max-page-size=0x1000
PROGRAM_pinion64 := pinion64
INSTALL_pinion64 := boot/pinion64
