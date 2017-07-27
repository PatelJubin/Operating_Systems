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
		fprintf(stderr, "Usage: ext2_rm <image file name> <path>\n");
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

	//Code for rm...
	ext2_group_desc *gp_desc = (struct ext2_group_desc *)disk + (EXT2_BLOCK_SIZE * EXT2_ROOT_INO);
	unsigned int bitmap_i = gp_desc->bg_inode_bitmap;
	unsigned int bitmap_b = gp_desc->bg_block_bitmap;

	struct ext2_inode *current_file = find_inode(argv[2], disk);

	if(current_file == NULL){
		printf("directory file/path doesn't exist");
		exit(ENOENT);
	}

	struct ext2_inode *parent_dir = find_inode(find_parent(argv[2]));

	if(parent_dir == NULL){
		printf("directory file/path doesn't exist");
		exit(ENOENT);
	}

	//Pointers for current block
	unsigned int *curr_block = parent_dir->i_block;
	unsigned int inode_num;

	int i = 0;
	for(i=0; i<12 && curr_block[i]; i++) {
		struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + EXT2_BLOCK_SIZE * curr_block[i]);

		int j = 0;
		while (j < EXT2_BLOCK_SIZE) {
			j += dir->rec_len;
			if (dir->file_type == EXT2_FT_REG_FILE && strncmp(file_name, dir->name, dir->name_len) == 0) {
        		struct ext2_dir_entry_2 *previous_dir = (struct ext2_dir_entry_2 *)(disk + EXT2_BLOCK_SIZE * curr_block[i]);;
      		}
		}
		dir = (struct ext2_dir_entry_2 *)((char *)dir + dir->rec_len);
	}

	curr_block = current_file->i_block;
	if (current_file->i_links_count == 0) {

	}
}