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
	struct ext2_group_desc *gp_desc = (struct ext2_group_desc *)disk + (EXT2_BLOCK_SIZE * EXT2_ROOT_INO);
	struct ext2_inode *inode_table = (struct ext2_inode *) disk + (EXT2_BLOCK_SIZE * gp_desc->bg_inode_table);
	struct ext2_inode *root_inode = &inode_table[EXT2_ROOT_INO - 1];

	//Initialize block and dir
	int block = 0;
	struct ext2_dir_entry_2 *dir;

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

unsigned int check_block_bitmap(unsigned char *disk){
	struct ext2_super_block *sp_block = (struct ext2_super_block *) (disk + EXT2_BLOCK_SIZE);
	struct ext2_group_desc *gp_desc = (struct ext2_group_desc *)(disk + (EXT2_ROOT_INO*EXT2_BLOCK_SIZE));
	//make sure there are free blocks before looping through bitmap
	if (sp_block->s_free_blocks_count > 0){
		char *blk_bitmap = (char *)(disk + (gp_desc->bg_block_bitmap * EXT2_BLOCK_SIZE));
		int i;
		int j;
		//loop through all of bitmap for free block
		for (i=0;i<16;i++){
			unsigned int bit = *(blk_bitmap + i);
			for(j=0;j<8;j++){
				//if the bit is not one then return that spot
				if(((bit>>j) & 1) == 0){
					return (i*8+j+1);
				}
			}
		}
	}
	//else return -1 if no free block found
	return -1;
}

unsigned int check_inode_bitmap(unsigned char *disk){
	struct ext2_super_block *sp_block = (struct ext2_super_block *) (disk + EXT2_BLOCK_SIZE);
	struct ext2_group_desc *gp_desc = (struct ext2_group_desc *)(disk + (EXT2_ROOT_INO*EXT2_BLOCK_SIZE));
	//check number of free inode
	if (sp_block->s_free_inode_count > 0){
		char *inode_bitmap = (char *)(disk + (gp_desc->bg_inode_bitmap * EXT2_BLOCK_SIZE));
		int i;
		int j;
		//loops through all of bitmap to find free spot
		for (i=0;i<4;i++){
			unsigned int bit = *(inode_bitmap + i);
			for(j=0;j<8;j++){
				//if the bit is not one then return that spot
				if(((bit>>j) & 1) == 0){
					return (i*8+j+1);
				}
			}
		}
	}
	//else return -1 if no free inode found
	return -1;
}

void set_inode_bit(unsigned char *disk){
	//get the free inode bit
	unsigned int check_inode = check_inode_bitmap(disk);
	struct ext2_group_desc *gp_desc = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE*EXT2_ROOT_INO));
	struct ext2_super_block *sp_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	//decrease the available # of free inodes
	sp_block->s_free_inode_count--;
	//set inode bitmap
	
}

void set_block_bit(unsigned char *disk){
	//get free block bit
	unsigned int check_block = check_block_bitmap(disk);
	struct ext2_group_desc *gp_desc = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE*EXT2_ROOT_INO));
	struct ext2_super_block *sp_block = (struct ext2_super_block *)(disk + EXT2_BLOCK_SIZE);
	
	//decrease available # of free blocks
	sp_block->s_free_blocks_count--;
	//set block bit
}
