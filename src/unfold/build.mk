# Build parameters for module 'unfold'

CFLAGS_unfold  := -std=c99
CFLAGS_unfold  += -pedantic -Wall -Wextra -Wno-unused-parameter -Werror
CFLAGS_unfold  += -m32 -mno-sse -mno-mmx -mno-sse2 -mno-sse3 -mno-3dnow
CFLAGS_unfold  += -ffreestanding
ASFLAGS_unfold := -felf32
LDFLAGS_unfold := -melf_i386
PROGRAM_unfold := unfold
INSTALL_unfold := boot/unfold
