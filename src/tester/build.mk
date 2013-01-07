# Build parameters for module 'tester'

CFLAGS_tester  := -std=c99 -MMD
CFLAGS_tester  += -pedantic -Wall -Wextra -Wno-unused-parameter -Werror
CFLAGS_tester  += -m64 -fPIE -mno-red-zone -fvisibility=hidden
CFLAGS_tester  += -mno-sse -mno-mmx -mno-sse2 -mno-sse3 -mno-3dnow
CFLAGS_tester  += -ffreestanding
CFLAGS_tester  += -Isrc/pinion64/include
CFLAGS_tester  += -O3 -fomit-frame-pointer
ASFLAGS_tester := -felf64
LDFLAGS_tester := -melf_x86_64 -z max-page-size=0x1000 -pie
LDFLAGS_tester += -Llib -lpinion
PROGRAM_tester := tester
INSTALL_tester := boot/tester
