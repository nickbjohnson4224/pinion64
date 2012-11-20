#include "internal.h"

struct mptable_float_ptr {
	uint32_t signature;
	uint32_t physaddr;
	uint8_t length;
	uint8_t spec_rv;
	uint8_t checksum;
	uint8_t feature[5];
} __attribute__((packed));

struct mptable_entry {
	uint8_t type;
} __attribute__((packed));

struct mptable {
	uint32_t signature;
	uint16_t base_length;
	uint8_t spec_rv;
	uint8_t checksum;
	uint8_t oem_id[8];
	uint8_t prod_id[12];
	uint32_t oem_table;
	uint16_t oem_table_size;
	uint16_t entry_count;
	uint32_t lapic_addr;
	uint16_t extended_length;
	uint8_t extended_checksum;
	uint8_t reserved;
	struct mptable_entry table[];
} __attribute__((packed));

struct mptable_cpu_entry {
	uint8_t type;
	uint8_t lapic_id;
	uint8_t lapic_version;
	uint8_t flags;
	uint32_t signature;
	uint32_t feature;
} __attribute__((packed));

struct mptable_bus_entry {
	uint8_t type;
	uint8_t id;
	uint8_t string[6];
} __attribute__((packed));

struct mptable_ioapic_entry {
	uint8_t type;
	uint8_t id;
	uint8_t ver;
	uint8_t flags;
	uint32_t addr;
} __attribute__((packed));

struct mptable_iointr_entry {
	uint8_t type;
	uint8_t intr_type;
	uint16_t flags;
	uint8_t bus;
	uint8_t irq;
	uint8_t ioapic;
	uint8_t ioapic_intn;
} __attribute__((packed));

void *ioapic_init(uint32_t *lapic) {
	extern struct mptable_float_ptr *find_mptable(void);
	uint32_t *ioapic;

	// find MP table
	struct mptable_float_ptr *mptable_ptr = find_mptable();

	if (!mptable_ptr) {
		log(ERROR, "no MP table found!");
		for(;;);
	}

	// parse MP table
	struct mptable *mptable = pmm_vaddr(mptable_ptr->physaddr);

	log(DEBUG, "MP table: rev %d, %d entries", mptable->spec_rv, mptable->entry_count);

	struct mptable_entry *entry = &mptable->table[0];

	for (uint32_t i = 0; i < mptable->entry_count; i++) {
		
		switch (entry->type) {
		case 0: {
			// processor entry
			struct mptable_cpu_entry *cpu = (void*) entry;
			log(DEBUG, "CPU (LAPIC id %x, signature %x)", cpu->lapic_id, cpu->signature);
			entry = (void*) ((uintptr_t) entry + 20);
			break;
		}
		case 1: {
			// bus entry
			struct mptable_bus_entry *bus = (void*) entry;
			uint8_t string[7];
			string[6] = 0;
			for (int j = 0; j < 6; j++) {
				string[j] = bus->string[j];
			}
			for (int j = 5; string[j] == ' '; j--) {
				string[j] = 0;
			}
			log(DEBUG, "BUS (id %x, type \"%s\")", bus->id, string);
			entry = (void*) ((uintptr_t) entry + 8);
			break;
		}
		case 2: {
			struct mptable_ioapic_entry *io = (void*) entry;
			log(DEBUG, "IOAPIC (id %x, version %d, addr %p", io->id, io->ver, pmm_vaddr(io->addr));
			entry = (void*) ((uintptr_t) entry + 8);
			break;
		}
		case 3:
		case 4: {
			struct mptable_iointr_entry *intr = (void*) entry;
			log(DEBUG, "INT (type %d bus %d irq %d ioapic %x intn %d)", intr->intr_type, intr->bus, intr->irq, intr->ioapic, intr->ioapic_intn);
			entry = (void*) ((uintptr_t) entry + 8);
			break;
		}

		}
	}

	ioapic = pmm_vaddr(0xFEC00000);	

	return ioapic;
}
