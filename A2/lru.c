#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

unsigned int curr_stamp;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	int page = 0;
	int i;
	unsigned long old = curr_stamp;

	for (i = 0; i < memsize; i++){
		if (coremap[i].timestamp < old){
			old = coremap[i].timestamp;
			page = i;
		}
	}
	
	return page;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	// setting the current time stamp
	coremap[p->frame >> PAGESHIFT].timestamp = curr_stamp;
	curr_stamp++;
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	curr_stamp = 0;
}
