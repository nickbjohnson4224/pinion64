.PHONY: clean tools clean-tools

all: make-all

##############################################################################
#
# Set up cross-compiler and build environment
#
##############################################################################

tools: clean-tools
	@ echo ""
	@ echo "Starting automatic cross-toolchain build. This may take a while depending on"
	@ echo "your Internet connection and processor speed. I would take this opportunity to"
	@ echo "mix up a batch of muffins if I were you."
	@ echo ""
	@ echo "The cross-toolchain will be completely contained within the tools/ directory. No"
	@ echo "superuser access is required."
	@ echo ""
	@ mkdir -p tools
	@ echo " WGET	tools/binutils-2.22.tar.bz2"
	@ wget -P tools http://ftp.gnu.org/gnu/binutils/binutils-2.22.tar.bz2
	@ echo " WGET	tools/gcc-4.7.0.tar.bz2"
	@ wget -P tools http://ftp.gnu.org/gnu/gcc/gcc-4.7.0/gcc-4.7.0.tar.bz2
	@ echo " WGET	tools/nasm-2.09.10.tar.bz2"
	@ wget -P tools http://www.nasm.us/pub/nasm/releasebuilds/2.09.10/nasm-2.09.10.tar.bz2
	@ echo " UNTAR	tools/binutils-2.22.tar.bz2"
	@ tar -xf tools/binutils-2.22.tar.bz2 -C tools
	@ rm tools/binutils-2.22.tar.bz2
	@ echo " UNTAR	tools/gcc-4.7.0.tar.bz2"
	@ tar -xf tools/gcc-4.7.0.tar.bz2 -C tools
	@ rm tools/gcc-4.7.0.tar.bz2
	@ echo " UNTAR	tools/nasm-2.09.10.tar.bz2"
	@ tar -xf tools/nasm-2.09.10.tar.bz2 -C tools
	@ rm tools/nasm-2.09.10.tar.bz2
	@ mkdir -p tools/build-binutils
	@ echo ""
	@ echo " CONFIGURING BINUTILS"
	@ echo ""
	@ cd tools/build-binutils && ../binutils-2.22/configure \
		--target=x86_64-elf --prefix=$(PWD)/tools --disable-nls
	@ echo ""
	@ echo " COMPILING BINUTILS"
	@ echo ""
	@ make -C tools/build-binutils all
	@ echo ""
	@ echo " INSTALLING BINUTILS"
	@ echo ""
	@ make -C tools/build-binutils install
	@ echo ""
	@ echo " CLEAN	tools/build-binutils tools/binutils-2.22"
	@ rm -rf tools/build-binutils tools/binutils-2.22
	@ mkdir -p tools/build-gcc
	@ echo ""
	@ echo " CONFIGURING GCC"
	@ echo ""
	@ export PATH=$PATH:$(PWD)/tools/bin
	@ cd tools/build-gcc && ../gcc-4.7.0/configure \
		--target=x86_64-elf --prefix=$(PWD)/tools --disable-nls \
		--enable-languages=c --without-headers
	@ echo ""
	@ echo " COMPILING GCC"
	@ echo ""
	@ make -C tools/build-gcc all-gcc
	@ echo ""
	@ echo " INSTALLING GCC"
	@ echo ""
	@ make -C tools/build-gcc install-gcc
	@ echo ""
	@ echo " CLEAN	tools/build-gcc tools/gcc-4.7.0"
	@ rm -rf tools/build-gcc tools/gcc-4.7.0
	@ echo ""
	@ echo " CONFIGURING NASM"
	@ echo ""
	@ cd tools/nasm-2.09.10 && ./configure --prefix=$(PWD)/tools
	@ echo ""
	@ echo " COMPILING NASM"
	@ echo ""
	@ make -C tools/nasm-2.09.10
	@ echo ""
	@ echo " INSTALLING NASM"
	@ echo ""
	@ make -C tools/nasm-2.09.10 install
	@ echo ""
	@ echo " CLEAN	tools/nasm-2.09.10"
	@ rm -rf tools/nasm-2.09.10
	
clean-tools:
	@ echo " CLEAN	tools/"
	@ - rm -r tools

##############################################################################
#
# Build components and install them to the system root
#
##############################################################################

COMPONENTS := unfold pinion64 tester

CC := ./tools/bin/x86_64-elf-gcc
LD := ./tools/bin/x86_64-elf-ld
AS := ./tools/bin/nasm

# build and install all packages into root/
make-all: skeleton $(COMPONENTS)

.PHONY: skeleton

# re-initialize root/ with a blank skeleton structure
skeleton:
	@ echo " CLEAN	root/"
	@ - rm -r root
	@ echo " CLONE	skel/ -> root/"
	@ cp -r skel root

# build templates
define template

OBJECTS_$(1) += $(patsubst src/%.s,build/%.o,$(shell find src/$(1) -name "*.s"))
OBJECTS_$(1) += $(patsubst src/%.c,build/%.o,$(shell find src/$(1) -name "*.c"))

build/$(1)/%.o: src/$(1)/%.c src/$(1)/build.mk
	@ mkdir -p `dirname $$@`
	@ echo " CC	"$$<
	@ $$(CC) -MMD -Isrc/$(1)/include $$(CFLAGS_$(1)) -c $$< -o $$@

build/$(1)/%.o: src/$(1)/%.s src/$(1)/build.mk
	@ mkdir -p `dirname $$@`
	@ echo " AS	"$$<
	@ $$(AS) $$(ASFLAGS_$(1)) $$< -o $$@

build/$(1)/$(PROGRAM_$(1)): $$(OBJECTS_$(1)) src/$(1)/$(1).ld src/$(1)/build.mk
	@ echo " LD	"$$@ $$(OBJECTS_$(1))
	@ $$(LD) $$(LDFLAGS_$(1)) -o $$@ $$(OBJECTS_$(1)) -Tsrc/$(1)/$(1).ld

$(1): build/$(1)/$(PROGRAM_$(1)) src/$(1)/build.mk
	@ echo " CP	"build/$(1)/$$(PROGRAM_$(1)) -> root/$$(INSTALL_$(1))
	@ cp build/$(1)/$$(PROGRAM_$(1)) root/$$(INSTALL_$(1))

-include $(patsubst %.o,%.d,%(OBJECTS_$(1)))

endef

$(foreach component,$(COMPONENTS),$(eval include src/$(component)/build.mk))
$(foreach component,$(COMPONENTS),$(eval $(call template,$(component))))

##############################################################################
#
# Create system images
#
##############################################################################

# create a new CD image in images/ from the contents of root/
cd-image: make-all
	@ mkdir -p images/
	@ echo " IMAGE	images/pinion.iso"
	@ mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot \
		-quiet -boot-load-size 4 -boot-info-table \
		-o images/pinion.iso root

# run the emulator to test the system
test: cd-image
	@ env test/run.sh

##############################################################################
#
# Clean up object files
#
##############################################################################

clean:
	@ echo " CLEAN	build/"
	@ - rm -r build
