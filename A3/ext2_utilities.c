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
	ext2_dir_entry_2 *dir= (struct ext2_dir_entry_2 *)disk + (EXT2_BLOCK_SIZE * root_inode->i_block[block];
char *rest;
char *split_path = strtok_r(path, "/", &rest);

while (split_path != NULL && block < 12 && root_inode->i_block[block]){

	while(){ 
		if (strncmp(split_path, dir->name, dir->name_len) == 0){
			root_inode = &inode_table[dir->inode]; 
			block = 0;
		}
		split_path = strtok_r(rest, "/", &rest);
		block ++;
	
	}
}


