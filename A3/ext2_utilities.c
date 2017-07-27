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
#include <ext2_utilities.h>


struct ext2_inode *find_inode (char *path, unsigned char *disk){
	ext2_group_desc *gp_desc = (struct ext2_group_desc *)disk + (EXT2_BLOCK_SIZE * EXT2_ROOT_INO);
	ext2_inode *inode_table = (struct ext2_inode *) disk + (EXT2_BLOCK_SIZE * gp_desc->bg_inode_table);
	ext2_inode *root_inode = &inode_table[EXT2_ROOT_INO];

	int block = 0;
	int block_size = 0;
	ext2_dir_entry_2 *dir;
	char *rest;
	char *curr_path = strtok_r(path, "/", &rest);


	while (curr_path != NULL && block < 12){

		unsigned char *curr_ptr = disk + (EXT2_BLOCK_SIZE * root_inode->i_block[block]);
		dir = (struct ext2_dir_entry_2 *)curr_ptr;

		//Loop until the we get to the end of the block
		while(block_size <= EXT2_BLOCK_SIZE){ 
			if (strncmp(curr_path, dir->name, dir->name_len) == 0){
				root_inode = &inode_table[dir->inode];
				//Since we go into a new dir, we have to reset block.
				block = 0;
				curr_path = strtok_r(rest, "/", &rest);
				break;
			}
			block_size += dir->rec_len;
		}
		block++;
	}

	//If we go through all blocks at some level without finidng the inode, the path does not exist.
	if (curr_path != NULL){
		return NULL;
	}else{
		return root_inode;
	}
}


