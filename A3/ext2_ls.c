#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <getopt.h>
#include "ext2.h"
#include <errno.h>
#include <string.h>

unsigned char *disk;

int main(int argc, char *argv[]) {
	//Check number of arguments
	if(argc != 3 || argc != 4){
		fprintf(stderr, "Usage: ext2_ls <image file name> <flag> <path>\n");
		exit(1);
	}
	//Open image
	int fd = open(argv[1], O_RDWR);

	//Map disk image
	disk = mmap(NULL, 128 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(disk == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

	int flag = 0;

	if (argc == 4) {
		if (strncmp(argv[2],"-a") == 0){
			flag = 1;
		}
	}

	//Code for ls...

	//Get inode (TO DO) struct ext2_inode *inode = find_inode(...);
	//TO BE REPLACED ONCE FIND_INODE IS IMPLEMENTED
	//KEEP IN MIND THE ARGV[i] DEPENDS ON IF THERE IS A FLAG
	struct ext2_inode *inode;

	//Pointers for current block
	unsigned int *curr_block = inode->i_block;

	//Each entry is saved here
	struct ext2_dir_entry_2 *entry;

	int i = 0;
	//Loop throught the 12 block pointers
	while (i < 12 && curr_block[i]){
		unsigned char *curr_ptr = disk + (curr_block[i] * EXT2_BLOCK_SIZE);
		//Max
		unsigned char *end = curr_ptr + EXT2_BLOCK_SIZE;
		//Get the first entry
		entry = (struct ext2_dir_entry_2 *)curr_ptr;
		while (curr_ptr < end){
			if ((flag) && (entry->name_len != 0)){
				printf("%s\n", entry->name);
			} else {
				if ((entry->name_len != 0) && (strncmp(entry->name, "..") != 0)){
					printf("%s\n", entry->name);
				}
			}
			curr_ptr += entry->rec_len;
			//Get the next entry
			entry = (struct ext2_dir_entry_2 *)curr_ptr;
		}
		i++;
	}
	return 0;
}