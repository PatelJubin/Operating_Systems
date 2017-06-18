#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

unsigned int fifo_front;

/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int fifo_evict() {
	unsigned int coremap_idx = fifo_front; //page at the front of the queue is selected to evict
	fifo_front = (fifo_front + 1) % memsize;
	return coremap_idx;
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {
	return; //to-do
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {
	fifo_front = 0;
}
