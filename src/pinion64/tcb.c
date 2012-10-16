#include "internal.h"

#define TCB_LIMIT 65536

static struct tcb ** tcb_table[TCB_LIMIT / 512];
static uint8_t tcb_table_lock;

struct tcb *tcb_new(void) {

	mutex_acquire(&tcb_table_lock);

	for (uint32_t i = 0; i < TCB_LIMIT; i++) {

		if (!tcb_table[i / 512]) {
			tcb_table[i / 512] = pge_alloc();
			for (int j = 0; j < 512; j++) {
				tcb_table[i / 512][j] = NULL;
			}
		}

		if (!tcb_table[i / 512][i % 512]) {
			struct tcb *tcb = tcb_alloc();
			if (!tcb) {

				mutex_release(&tcb_table_lock);
				return NULL;
			}

			tcb_table[i / 512][i % 512] = tcb;

			tcb->mutex = 1;
			tcb->state = TSTATE_RE;
			tcb->id = i;

			mutex_release(&tcb_table_lock);
			return tcb;
		}

	}

	mutex_release(&tcb_table_lock);
	return NULL;
}
