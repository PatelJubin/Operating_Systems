#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

#define MAXLINE 256

extern int memsize;

extern int debug;

extern struct frame *coremap;

int tr_size = 0;
static int count_idx;
int *vaddr_list;//list of virtual address in trace file
int *maddr_list;//list of virtual address in memory
int *used_next;
extern char *tracefile;

/* Page to evict is chosen using the optimal (aka MIN) algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	//initializing variables that will be needed
	int evict_frame = 0;
	int i;
	int k = 0;
	int j = count_idx;
	int max_used_next = 0;

	//loop through the vaddr_list in memeory
	//while populating our used_next array
	for (i = 0; i < memsize; i++) {
		for (j = count_idx; j < tr_size; j++) {
			//if we found the virtual address, we want
			//to store the next used index.
			if (vaddr_list[j] == maddr_list[i]) {
				used_next[i] = j;
				break;
			}
		}
		//if we cant find the virtual address, return j
		//which will be evicted.
		if (j == tr_size){
		return i;
		}
	}
	//swap out our furthest index and frame if our current frame's
	//virtual address has a longer distance
	for (k = 0; k < memsize; k++) {
		if (used_next[k] > max_used_next) {
			evict_frame = k;
			max_used_next = used_next[k];
		}
	}
	return evict_frame;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	//store the virtual addresses into a list
	maddr_list[p -> frame >> PAGE_SHIFT] = vaddr_list[count_idx++];
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	//initializing variables
	int i = 0; //loop variable for populating vaddr_list at the end
	FILE *tfp;
    char buf[MAXLINE];
    addr_t vaddr = 0;
    char type;
	used_next = malloc(memsize * sizeof(int));
	maddr_list = malloc(memsize * sizeof(int));

	//error handling and initializing tracefile, taken from sim.c file
	if((tfp = fopen(tracefile, "r")) == NULL) {
		perror("Error opening tracefile:");
		exit(1);
	}

	//read the trace file to get the size we need for memory allocation
	while(fgets(buf, MAXLINE, tfp) != NULL){
		tr_size++;
	}

	//set our vaddr_list after determining the size of tracefile
	vaddr_list = malloc(tr_size * sizeof(int));

	//we want to read from the beginning again since the while loop before read
	//through the file and got to the end
	rewind(tfp);

	//taken from sim.c we parse through our tracefile and populate our vaddr_list.
	while(fgets(buf, MAXLINE, tfp) != NULL){
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			vaddr_list[i++] =  vaddr;
		}
	}
}

