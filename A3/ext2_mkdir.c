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
	char *dir_name = argv[2]+(strlen(parent_dir));
	
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
		printf("no space in bitmap");
		exit(1);
	}else
	{
		set_inode_bit(disk);
		set_block_bit(disk);
	}
	
	struct ext2_inode *inode = &(inode_table[check_inode -1]);
	struct ext2_dir_entry_2 *dir = (struct ext2_dir_entry_2 *)(disk + (check_blk *EXT2_BLOCK_SIZE));	
	//setting up inode for the new inode
	inode->i_mode = EXT2_S_IFDIR;
	inode->i_links_count = 1;
	inode->i_size = EXT2_BLOCK_SIZE;
	inode->i_blocks = 1;
	inode->i_block[0] = check_inode;
	//setting up directory entry for the new directory
	dir->file_type = EXT2_FT_DIR;
	dir->name_len = 1;
	strncpy(dir->name, ".", 1);
	dir->rec_len = 12;
	dir->inode = check_inode;
	
	//setting up the parent's directory
	struct ext2_dir_entry_2 *p_dir = (struct ext2_dir_entry_2 *)((char *) (dir + 12));
	//want to use our earlier parent inode to set the directory
	//cast the p_inode's i_block with ext2_dir_entry_2 to get inode for directory
	p_dir->inode = ((struct ext2_dir_entry_2 *)(disk + (p_inode->i_block[0] * EXT2_BLOCK_SIZE )))->inode;
	p_dir->name_len = 2;
	strncpy(p_dir->name, "..", 2);
	//rec len is block size - rec len of the "." directory which is 12
	p_dir->rec_len = EXT2_BLOCK_SIZE - 12;
	p_dir->file_type = EXT2_FT_DIR;
	//increase link counts
	p_inode->i_links_count++;
	
	//now we need to place the "." directory into the blocks of the parent directory
	//we need to loop through the i_block to find free space, if none then we need to create some
	int dir_set = 0;
	int i;
	//4B algined
	int dir_offset = 4 - (strlen(dir_name) % 4);
	for(i = 0; i < 12 && p_inode->i_block[i] ; i++)
	{
		int block_size;
		struct ext2_dir_entry_2 *d_entry = (struct ext2_dir_entry_2 *)(disk +(p_inode->i_block[i] * EXT2_BLOCK_SIZE));
		for (block_size = 0; block_size < EXT2_BLOCK_SIZE; block_size+=d_entry->rec_len){
			int n_offset = 4 - (d_entry->name_len % 4);
			//check to see if there is enough space for new directory
			//add 16 at the end cause size of static fields are 8 in total so minimum rec_len for directory is
			//8 + amount in name array, so we do this twice, thus 16
			if(d_entry->rec_len >= (strlen(dir_name) + d_entry->name_len + n_offset + dir_offset + 16)){
				//set up the directory fields if free space found
				unsigned int rec_len = d_entry->name_len + n_offset + 8;
				p_dir = (struct ext2_dir_entry_2 *)((char *)(d_entry+rec_len));
				p_dir->inode = check_inode;
				p_dir->rec_len = d_entry->rec_len - rec_len;
				p_dir->name_len = strlen(dir_name);
				strncpy(p_dir->name, dir_name, p_dir->name_len);
				p_dir->file_type = EXT2_FT_DIR;
				//set the rec_len for d_entry as calculated
				d_entry->rec_len = rec_len;
				dir_set = 1;
				break;
			}
			d_entry = (struct ext2_dir_entry_2 *)((char *)(d_entry + d_entry->rec_len));

		}
		if(dir_set)
		{
			break;
		}
	}
	//if we didnt set up the directory fields we need to a new block to do so
	if(dir_set == 0){
		//find free block
		p_inode->i_block[i]	= check_block_bitmap(disk);
		if(p_inode->i_block[i] == -1){
			printf("error no space left");
			exit(1);
		}
		struct ext2_dir_entry_2 *p2_dir = (struct ext2_dir_entry_2 *)(disk + (p_inode->i_block[i] * EXT2_BLOCK_SIZE));
		//set block bit
		set_block_bit(disk);
		//set up fields of directory
		p2_dir->name_len = strlen(dir_name);
		strncpy(p2_dir->name,dir_name, p2_dir->name_len);
		p2_dir->inode = check_inode;
		p2_dir->rec_len = EXT2_BLOCK_SIZE;
		p2_dir->file_type=EXT2_FT_DIR;
		
		//update parents inode
		p_inode->i_size += EXT2_BLOCK_SIZE;
		p_inode->i_blocks += 2;	
	}
	return 0;
}
	

