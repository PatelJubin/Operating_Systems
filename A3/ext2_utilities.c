
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
	/*
	We get the root inode by getting the descriptor for the root, then getting the inode_table entry for
	the root.
	*/
	ext2_group_desc *gp_desc = (struct ext2_group_desc *)disk + (EXT2_BLOCK_SIZE * EXT2_ROOT_INO);
	ext2_inode *inode_table = (struct ext2_inode *) disk + (EXT2_BLOCK_SIZE * gp_desc->bg_inode_table);
	ext2_inode *root_inode = &inode_table[EXT2_ROOT_INO - 1];

	//Initialize block and dir
	int block = 0;
	ext2_dir_entry_2 *dir;

	//We will split the path until we get to NULL
	char *rest;
	char *curr_path = strtok_r(path, "/", &rest);

	//Loop through the blocks of the current dir
	while (curr_path != NULL && block < 12){
		//Changes which BLOCK is pointed to
		unsigned char *curr_ptr = disk + (EXT2_BLOCK_SIZE * root_inode->i_block[block]);
		//Max
		unsigned char *end = curr_ptr + EXT2_BLOCK_SIZE;
		//Get the entry
		dir = (struct ext2_dir_entry_2 *)curr_ptr;

		//Loop until the we get to the end of the block
		while(curr_ptr < end){ 
			if (strncmp(curr_path, dir->name, dir->name_len) == 0){
				root_inode = &inode_table[dir->inode - 1];
				//Since we go into a new dir, we have to reset block.
				block = 0;
				curr_path = strtok_r(rest, "/", &rest);
				break;
			}
			//Go to the next entry in the current block
			curr_ptr += dir->rec_len;
			dir = dir = (struct ext2_dir_entry_2 *)curr_ptr;
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

char *find_parent(char *path){
	//copy path to use for strtok
	char *copy_path = malloc(sizeof(char)*(strlen(path)+1));
	strncpy(copy_path, path, strlen(path));
	//split path on / to check if at root
	char *split_path = strtok(copy_path, "/");
	//get the string after the last "/" which is the last entry in the path
	//this includes the "/" as well
	char *last_dir = strrchr(path, '/');
	char *parent = NULL;
	
	//if split path is null return root
	if (split_path == NULL){
		return "/";
	}
	else
	{
		//copy the parent path into parent and return
		parent = strndup(path, last_dir - path);
		return parent;
	}
	
}


