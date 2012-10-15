/*
 * Copyright (C) 2012 Nick Johnson <nickbjohnson4224 at gmail.com>
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdint.h>
#include <stddef.h>

#include "multiboot.h"
#include "object.h"
#include "mmap.h"
#include "elf.h"
#include "log.h"

const uint32_t pinion_phys = 0x00200000;
const uint64_t pinion_base = 0xFFFFFFFFC0200000ULL;

extern void init_enter_long(uint64_t entry);

static uint64_t load_pinion(const struct elf_ehdr *elf);
static uint64_t load_pinion64(const struct elf64_ehdr *elf64);

void init(const struct multiboot_header *mboot, uint32_t magic) {

	log(INIT, "unfold (pinion loader) starting...");

	if (magic == 0x2BADB002) {
		// is multiboot-compliant loader
		init_multiboot(mboot);
	}
	else {
		// non-multiboot loader; panic
		log(ERROR, "invalid multiboot magic number: %x", magic);
		return;
	}

	if (init_mmap()) {
		log(ERROR, "failed to initialize memory map");
		return;
	}

	if (init_object_list()) {
		log(ERROR, "failed to construct object list");
		return;
	}

	if (!pinion_image) {
		log(ERROR, "no pinion image found");
		return;
	}

	log(INIT, "pinion image found at 0x%x size %d B",
		(uint32_t) pinion_image->base, pinion_image->size);

	if (pinion_image->base >= (1ULL << 32)) {
		log(ERROR, "pinion image out of 32-bit range");
		return;
	}

	uint64_t entry = load_pinion((void*) (uint32_t) pinion_image->base);

	log(INIT, "pinion entry point at %x%x", (uint32_t) (entry >> 32), (uint32_t) entry);

	init_enter_long(entry);
}

static uint64_t load_pinion(const struct elf_ehdr *elf) {

	// choose base address for Pinion
	log(INIT, "loading pinion at base address %x%x (phys %x)",
		(uint32_t) (pinion_base >> 32), (uint32_t) pinion_base,
		pinion_phys);

	// check ELF header
	if (elf->e_ident[EI_MAG0] != 0x7F ||
		elf->e_ident[EI_MAG1] != 'E' ||
		elf->e_ident[EI_MAG2] != 'L' ||
		elf->e_ident[EI_MAG3] != 'F') {

		log(ERROR, "pinion image is not a valid ELF binary");
		return 0ULL;
	}

	// check ELF type
	if (elf->e_ident[EI_CLASS] == ELFCLASS64 && 
		elf->e_ident[EI_DATA] == ELFDATA2LSB &&
		elf->e_machine == EM_AMD64) {

		return load_pinion64((const void*) elf);
	}

	log(ERROR, "pinion image is unknown ELF binary type");
	return 0ULL;
}

static uint64_t load_pinion64(const struct elf64_ehdr *elf64) {
	const uint8_t *binary = (void*) elf64;

	for (uint32_t i = 0; i < elf64->e_phnum; i++) {
		const struct elf64_phdr *phdr = 
			(const void*) &binary[elf64->e_phoff + elf64->e_phentsize * i];

		uint64_t base = phdr->p_vaddr + pinion_base;
		uint32_t phys = phdr->p_vaddr + pinion_phys;

		if (phdr->p_type == 1) {
			log(DEBUG, "LOAD %x%x (phys %x) from %x size %d", 
				(uint32_t) (base >> 32), (uint32_t) base, phys, 
				&binary[phdr->p_offset], phdr->p_filesz);

			for (uint32_t j = 0; j < phdr->p_memsz; j++) {
				if (j < phdr->p_filesz) {
					((uint8_t*) phys)[j] = binary[phdr->p_offset + j];
				}
				else {
					((uint8_t*) phys)[j] = 0;
				}
			}
		}
	}

	return elf64->e_entry + pinion_base;
}
