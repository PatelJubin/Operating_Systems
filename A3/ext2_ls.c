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
	if(argc != 3){
		fprintf(stderr, "Usage: ext2_ls <image file name> <path>\n");
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

	//Code for ls...

	//Get inode (TO DO) struct ext2_inode *inode = find_inode(...);
	//TO BE REPLACED ONCE FIND_INODE IS IMPLEMENTED
	struct ext2_inode *inode;

	//Pointers for current block
	unsigned int *curr_block = inode->i_block;

	//Each entry is saved here
	struct ext2_dir_entry_2 *entry;

	//Name of directory is saved here before printing
	char name[256];

	int i = 0;
	while (i < 15 && curr_block[i]){
		unsigned int curr_size = 0;
		entry = (struct ext2_dir_entry_2 *) (disk + (curr_block[i] * EXT2_BLOCK_SIZE));
		while (curr_size <= EXT2_BLOCK_SIZE){
			strnpy(name, entry->name, entry->name_len);
			printf("%s\n", name);
			curr_size += entry->rec_len;
		}
	}
	return 0;
}