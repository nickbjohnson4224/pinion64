// Copyright (C) 2012-2013 Nick Johnson <nickbjohnson4224 at gmail.com>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "internal.h"

// ELF header

#define EI_MAG0		0
#define EI_MAG1		1
#define EI_MAG2		2
#define EI_MAG3		3
#define EI_CLASS	4
#define EI_DATA		5
#define EI_VERSION	6
#define EI_PAD		7
#define EI_NIDENT	16

struct elf64_ehdr {
	uint8_t  e_ident[EI_NIDENT];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint64_t e_entry;
	uint64_t e_phoff;
	uint64_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

#define ET_NONE		0
#define ET_REL		1
#define ET_EXEC 	2
#define ET_DYN		3

#define EM_386  	3

#define ELFMAG0		0x7F
#define ELFMAG1		'E'
#define ELFMAG2		'L'
#define ELFMAG3		'F'

// ELF program header

struct elf64_phdr {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
};

#define PT_NULL 	0
#define PT_LOAD 	1
#define PT_DYNAMIC 	2
#define PT_INTERP 	3
#define PT_NOTE 	4
#define PT_SHLIB 	5
#define PT_PHDR		6

#define PF_R	0x1
#define PF_W	0x2
#define PF_X	0x4

// ELF dynamic entry

struct elf64_dyn {
	uint64_t d_tag;
	uint64_t d_val;
};

#define DT_NULL		0
#define DT_NEEDED	1
#define DT_PLTRELSZ	2
#define DT_PLTGOT	3
#define DT_HASH		4
#define DT_STRTAB	5
#define DT_SYMTAB	6
#define DT_RELA		7
#define DT_RELASZ	8
#define DT_RELAENT	9
#define DT_STRSZ	10
#define DT_SYMENT	11
#define DT_INIT		12
#define DT_FINI		13
#define DT_SONAME	14
#define DT_RPATH	15
#define DT_SYMBOLIC	16
#define DT_REL		17
#define DT_RELSZ	18
#define DT_RELENT	19
#define DT_PLTREL	20
#define DT_DEBUG	21
#define DT_TEXTREL	22
#define DT_JMPREL	23

// ELF dynamic symbols

struct elf64_sym {
	uint32_t st_name;
	uint8_t  st_info;
	uint8_t  st_other;
	uint16_t st_shndx;
	uint64_t st_value;
	uint64_t st_size;
} __attribute__((packed));

// ELF relocations

struct elf64_rela {
	uint64_t r_offset;
	uint32_t r_type;
	uint32_t r_sym;
	int64_t  r_addend;
};

#define R_X86_64_JUMP_SLOT	7
#define R_X86_64_RELATIVE	8

//

struct dynamic {

	// string table
	const char *str;

	// symbol table
	const void *sym;
	size_t syment;

	// API revision
	uint8_t apirev;

	// relocation table (RELA only)
	const void *rel;
	const void *jmprel;
	size_t relsz;
	size_t jmprelsz;
	size_t relent;

	// PLT/GOT
	const void *pltgot;
};

//

static int strcmp(const char *a, const char *b) {
	
	while (*a && *b) {
		if (*(a++) != *(b++)) return 1;
	}

	return 0;
}

static void memcpy(uint8_t *d, const uint8_t *s, size_t size) {
	
	for (size_t i = 0; i < size; i++) {
		d[i] = s[i];
	}
}

static void memclr(uint8_t *d, size_t size) {
	
	for (size_t i = 0; i < size; i++) {
		d[i] = 0;
	}
}

static void relocate(uint64_t kernel_base, struct dynamic *d, const struct elf64_rela *r) {

	const struct elf64_sym *sym = (const void*) ((uintptr_t) d->sym + (d->syment * r->r_sym));
	uint64_t *ptr = NULL;
	uint64_t value = 0;

	// calculate relocation constants
	switch (r->r_type) {
	case R_X86_64_JUMP_SLOT:
		ptr = (void*) (kernel_base + r->r_offset);
		value = api_resolve(&d->str[sym->st_name]) + r->r_addend;
		break;
	case R_X86_64_RELATIVE:
		ptr = (void*) (kernel_base + r->r_offset);
		value = kernel_base + r->r_addend;
		break;
	default:
		log(DEBUG, "unhandled dynamic relocation type %d", r->r_type);
		return;
	}

	// perform relocation
	*ptr = value;
}

static void fixup_kernel(uint64_t kernel_base, struct elf64_dyn *dynamic) {

	// collect dynamic linking information
	struct dynamic d = {
		.str = NULL,
		.sym = NULL,
		.syment = 0,
		.apirev = 0,
		.rel = NULL,
		.jmprel = NULL,
		.relsz = 0,
		.jmprelsz = 0,
		.relent = 24,
		.pltgot = NULL,
	};

	size_t needed_off = 0;
	for (size_t i = 0; dynamic[i].d_tag; i++) {
		uint64_t val = dynamic[i].d_val;

		switch (dynamic[i].d_tag) {
		case DT_STRTAB:   d.str = (const char*) (kernel_base + val);    break;
		case DT_RELA:     d.rel = (void*) (kernel_base + val);          break;
		case DT_RELASZ:   d.relsz = val;                                break;
		case DT_RELAENT:  d.relent = val;                               break;
		case DT_PLTRELSZ: d.jmprelsz = val;                             break;
		case DT_JMPREL:   d.jmprel = (void*) (kernel_base + val);       break;
		case DT_NEEDED:   needed_off = val;                             break;
		case DT_SYMTAB:   d.sym = (const void*) (kernel_base + val);    break;
		case DT_SYMENT:   d.syment = val;                               break;
		case DT_PLTGOT:   d.pltgot = (const void*) (kernel_base + val); break;
		}
	}

	// check requested API revision
	const char *needed = &d.str[needed_off];
	const char *match = "libpinion.so.";
	
	for (size_t i = 0; i < 13; i++) {
		if (needed[i] != match[i]) {
			log(ERROR, "unrecognized API string %s", needed);
			break;
		}
	}

	d.apirev = ((needed[13] - '0') << 4) + needed[15] - '0';

	// perform relocations
	if (d.jmprel) {
		log(INFO, "JMPREL at %p; %d entries", d.jmprel, d.jmprelsz / d.relent);

		for (size_t i = 0; i < d.jmprelsz; i += d.relent) {
			relocate(kernel_base, &d, (const void*) ((uintptr_t) d.jmprel + i));
		}
	}
}

void load_kernel(struct unfold64_objl *objl) {

	// search for kernel
	const char *kernel_name = config_read("loader", "image");

	if (!kernel_name) {
		log(ERROR, "no kernel image specified");
		for(;;);
	}

	size_t i;
	for (i = 0; i < objl->count; i++) {
		if (!strcmp(objl->entry[i].name, kernel_name)) {
			// found kernel
			log(INFO, "found kernel object at %p", objl->entry[i].base);
			break;
		}
	}

	// check kernel
	struct elf64_ehdr *kernel = (void*) objl->entry[i].base;

	if (kernel->e_ident[EI_MAG0] != ELFMAG0 ||
		kernel->e_ident[EI_MAG1] != ELFMAG1 ||
		kernel->e_ident[EI_MAG2] != ELFMAG2 ||
		kernel->e_ident[EI_MAG3] != ELFMAG3) {

		log(ERROR, "kernel is not valid ELF");
		for(;;);
	}

	// load kernel
	uint64_t kernel_base = config_read_as_ptr("loader", "base", 0xFFFFFFFF00000000ULL);
	struct elf64_dyn *dynamic = NULL;

	for (size_t i = 0; i < kernel->e_phnum; i++) {
		struct elf64_phdr *phdr = (void*) ((uintptr_t) kernel + kernel->e_phoff + i * kernel->e_phentsize);

		if (phdr->p_type == PT_LOAD || phdr->p_type == PT_DYNAMIC) {
			log(INFO, "loading at %p size %d", kernel_base + phdr->p_vaddr, phdr->p_filesz);

			for (size_t offset = 0; offset < phdr->p_memsz; offset += 0x1000) {
				pcx_set_frame(NULL, kernel_base + phdr->p_vaddr + offset, 0x1000, 
					pcx_get_trans(NULL, (uint64_t) pge_alloc()), PF_PRES | PF_WRITE | PF_GLOBL);
			}

			memcpy((void*) (kernel_base + phdr->p_vaddr), 
				(void*) ((uintptr_t) kernel + phdr->p_offset), phdr->p_filesz);
			memclr((void*) (kernel_base + phdr->p_vaddr + phdr->p_filesz), 
				phdr->p_memsz - phdr->p_filesz);
		}

		if (phdr->p_type == PT_DYNAMIC) {
			dynamic = (void*) ((uintptr_t) kernel + phdr->p_offset);
		}
	}

	fixup_kernel(kernel_base, dynamic);

	// create kernel stack
	uint64_t kernel_stack = config_read_as_ptr("loader", "stack", 0xFFFFFFFF80000000ULL);
	pcx_set_frame(NULL, kernel_stack - 0x1000, 0x1000, 
		pcx_get_trans(NULL, (uint64_t) pge_alloc()),
		PF_PRES | PF_WRITE | PF_GLOBL);

	// set current thread state to kernel entry
	struct tcb *tcb = ccb_get_self()->active_tcb;

	tcb->rip = kernel_base + kernel->e_entry;
	tcb->rsp = kernel_stack;
}
