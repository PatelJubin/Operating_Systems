struct ext2_inode *find_inode(char *path, unsigned char *disk);
char *find_parent(char *path);
unsigned int check_block_bitmap(unsigned char *disk);
unsigned int check_inode_bitmap(unsigned char *disk);
void set_inode_bit(unsigned char *disk);
void set_block_bit(unsigned char *disk);
void unset_block_bit(unsigned int bmap_block, unsigned char *disk);
void unset_inode_bit(unsigned int bmap_inode, unsigned char *disk);