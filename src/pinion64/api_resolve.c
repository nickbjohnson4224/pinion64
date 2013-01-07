#include "internal.h"

static int strcmp(const char *a, const char *b) {
	
	while (*a && *b) {
		if (*(a++) != *(b++)) return 1;
	}

	return 0;
}

#define APICATCH(NAME) \
do {\
	if (!strcmp(symbol, "pinion_" #NAME)) {\
		return pinion_load_addr + (uint64_t) pinion_##NAME;\
	}\
} while (0) 

extern void pinion_thread_yield(void);

uint64_t api_resolve(uint8_t apirev, const char *symbol) {
	static uint64_t pinion_load_addr = 0;
	
	if (!pinion_load_addr) pinion_load_addr = pmm_get_pinion_load_addr();

	APICATCH(thread_yield);

	return 0;
}
