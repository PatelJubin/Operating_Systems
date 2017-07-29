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

unsigned char *disk;

int main(int argc, char *argv[]) {
	//Check number of arguments
	if(argc != 3){
		fprintf(stderr, "Usage: ext2_mkdir <image file name> <path>\n");
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
	
	char *parent_dir = find_parent(argv[2]);
	struct ext2_group_desc *gp_desc = (struct ext2_group_desc *)(disk + (EXT2_BLOCK_SIZE * EXT2_ROOT_INO));
	struct ext2_inode *inode_table = (struct ext2_inode *)(disk + (EXT2_BLOCK_SIZE * gp_desc->bg_inode_table));
	struct ext2_inode *p_inode = find_inode(parent_dir, disk);
	
	//error checking
	if(p_inode == NULL){
		printf("directory path doesn't exist");
		exit(ENOENT);
	}
	if(find_inode(argv[2], disk) != NULL){
		printf("file/directory exists");
		exit(EEXIST);
	}
	unsigned int check_inode = check_inode_bitmap(disk);
	unsigned int check_blk = check_block_bitmap(disk);
	
	if((check_inode == -1) || (check_blk == -1)){
		prinf("no space in bitmap");
		exit();
	}else
	{
		set_inode_bit(disk);
		set_block_bit(disk);
		struct ext2_inode *inode = inode = &(inode_table[check_inode -1]);
	}
	
	//code for mkdir
	
}