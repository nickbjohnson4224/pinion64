#include "internal.h"

#define CCB_LIMIT 16

static struct ccb * ccb_table[CCB_LIMIT];
static uint8_t ccb_alloc_lock;

struct ccb *ccb_new(void) {

	mutex_acquire(&ccb_alloc_lock);

	for (int i = 0; i < CCB_LIMIT; i++) {
		if (!ccb_table[i]) {
			ccb_table[i] = ccb_alloc();
			ccb_table[i]->id = i;
			mutex_release(&ccb_alloc_lock);
			return ccb_table[i];
		}
	}

	mutex_release(&ccb_alloc_lock);

	return NULL;
}

struct ccb *ccb_get(uint16_t core_id) {
	return ccb_table[core_id];
}

struct ccb *ccb_get_self(void) {
	// TODO - actual multiprocessor support 6_9
	return ccb_table[0];
}
