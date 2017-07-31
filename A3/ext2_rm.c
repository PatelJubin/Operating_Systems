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
#include "ext2_utilities.h"

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
	struct ext2_group_desc *gp_desc = (struct ext2_group_desc *)(disk + EXT2_BLOCK_SIZE * EXT2_ROOT_INO);

	// Get the inode for the current file
	struct ext2_inode *current_file = find_inode(argv[2], disk);

	if(current_file == NULL) {
		printf("directory file/path doesn't exist");
		exit(ENOENT);
	}

	current_file->i_links_count--;

	// Parent dir link
	struct ext2_inode *parent_dir = find_inode(find_parent(argv[2]), disk);

	if(parent_dir == NULL){
		printf("directory file/path doesn't exist");
		exit(ENOENT);
	}

	// Pointer to current block
	unsigned int *curr_block = parent_dir->i_block;

	// Store the file name
	char file_name[512];

	// Keep track of inodes
	unsigned int inode_num;

	int i, k, j =0;
	while(i<12 && curr_block[i]) {
		struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + EXT2_BLOCK_SIZE * curr_block[i]);

		int size = 0;
		while (size < EXT2_BLOCK_SIZE) {
			size = dir->rec_len + size;

			// if entry is a file
			if (dir->file_type == EXT2_FT_REG_FILE && strncmp(file_name, dir->name, dir->name_len) == 0) {
        		struct ext2_dir_entry_2 *previous_dir = (struct ext2_dir_entry_2 *)(disk + EXT2_BLOCK_SIZE * curr_block[i]);
        		
        		// set previous dir
        		while((struct ext2_dir_entry_2 *)((char *)previous_dir + previous_dir->rec_len) != dir) {
						previous_dir = (struct ext2_dir_entry_2 *)((char *)previous_dir + previous_dir->rec_len);
				}

        		previous_dir->rec_len = dir->rec_len + previous_dir->rec_len;
        		inode_num = dir->inode;
      		} else {
      			return EISDIR; // entry is a dir
      		}
		}
		size = dir->rec_len - size;
		dir = (struct ext2_dir_entry_2 *)((char *)dir + dir->rec_len);
		i++;
	}

	curr_block = current_file->i_block;

	// check if no links exists and unset bitmaps
	if (current_file->i_links_count == 0) {

		//unset inode bitmap
		unset_inode_bit(inode_num, disk);

		//unset block bitmap
		unsigned int *singly_pt = (unsigned int *) (disk + curr_block[i] * EXT2_BLOCK_SIZE);
		unset_block_bit(singly_pt[j], disk);
		
		// Get the free blocks
		int blocks;
        for (k = 0; k < 12 && curr_block[j]; k++) {
        	current_file->i_block[k] = 0;
        	blocks++;
        }

        // Update the free block and free inode counts
        gp_desc->bg_free_blocks_count += blocks;
        gp_desc->bg_free_inodes_count++;

        //set i_size to 0
        current_file->i_size = 0;

	}
	return 0;
}