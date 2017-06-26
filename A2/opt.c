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

extern char *tracefile;
unsigned int tr_size;
//using struct addr_t from pagetable.h which is just unsigned long
addr_t *virt_addr_list; //list of virtual address in trace file
addr_t *mem_addr_list; //list of virtual address in memory
unsigned int *used_next;
unsigned long count_idx;

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	
	return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	int frame;
	frame = (p->frame >> PAGE_SHIFT);
	p->frame.timestamp = counter;
	counter++;
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	//initializing variables
	int i = 0; //loop variable for populating virt_addr_list at the end
	FILE *tfp;
	char type;
	addr_t vaddr = 0;
	char buf[MAXLINE];
	used_next = malloc(memsize * sizeof(int));
	mem_addr_list = malloc(memsize * sizeof(int));
	
	
	//error handling and initializing tracefile, taken from sim.c file
	if(tracefile != NULL) {
		if((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
		}
	}
	
	//read the trace file to get the size we need for memory allocation
	while(fgets(buf, MAXLINE, tfp) != NULL) {
		tr_size++;
	}
	
	//set our virt_addr_list after determining the size of tracefile
	virt_addr_list = malloc(tr_size * sizeof(int));
	
	//we want to read from the beginning again since the while loop before read
	//through the file and got to the end
	rewind(tfp);
	
	
	//taken from sim.c we parse through our tracefile and populate our virt_addr_list.
	while(fgets(buf, MAXLINE, infp) != NULL) {
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			virt_addr_list[i] = vaddr;
			i++;
		}

}

