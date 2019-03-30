#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "types.h"

#define FNAME_LEN 		32
#define BITS_IN_A_BYTE 	8 					// <-- For Peter Clark
#define DENTRY_RES 		24
#define BOOT_BLOCK_RES 	52
#define NUMFILES 		63
#define MAX_DBLOCKS		1023
#define FOURKB_BYTES	4096
#define FOURKB_BITS 	FOURKB_BYTES*BITS_IN_A_BYTE
#define INODE_ADDR		BOOT_BLOCK_ADDR + FOURKB_BYTES
#define BYTES_IN_4KB	4096

#define MAX_FNAME_LEN   32
#define BLOCK_SIZE      4096
#define NULL            0

#define USED_MASK       0x1
#define READ_MASK       0x2
#define WRITE_MASK      0x4

/* The struct for a dentry (file metadata);
must be exactly 64B; holds the name of the file,
the type, the index to its corresponding inode,
and reserved space.*/
typedef struct dentry{
	uint8_t file_name[FNAME_LEN];
	uint32_t file_type;
	uint32_t inode_num;
	uint8_t res[DENTRY_RES];
} dentry_t;

/* The boot block, containing metadata on
the whole filesystem. Exactly 4KB large,
contains counts of dentrues, inodes, datablocks,
reserved space, and a list of dentries after.*/
typedef struct boot_block{
	uint32_t dentry_count;
	uint32_t inode_count;
	uint32_t datablock_count;
	uint8_t reserved[BOOT_BLOCK_RES];
	dentry_t dentries[NUMFILES];
} boot_block_t;

/* The data structure for an inode. Contains
how many bytes exist in there as well as the ids
to the data blocks holding said bytes.*/
typedef struct inode{
	uint32_t len_b;
	uint32_t data_ids[MAX_DBLOCKS];
} inode_t;

/*Just a block of 4KB of data.*/
typedef struct data_block{
	uint8_t data[BYTES_IN_4KB];
} data_block_t;

/*A data structure to hold info on status of our current file*/
// typedef struct curr_file{
// 	uint8_t open;
// 	uint32_t bytes_read;
// 	dentry_t dentry;
// }curr_file_t;


/*A data structure to hold info on status of our current directory*/
typedef struct curr_dir{
	uint8_t open;
	uint32_t bytes_read;
	uint32_t curr_idx;
	dentry_t dentry;
}curr_dir_t;


//Our pointers to the data
boot_block_t* boot_block_ptr;
inode_t* inodes;
data_block_t* datas;

//Our bookkeeping data structures
// curr_file_t currfile;
curr_dir_t currdir;



// typedef union file_block{
// 	boot_block_t boot_block;
// 	inode_t inode;
// 	uint32_t data_block			:FOURKB_BITS;
// } file_block_t;

//Helper functions

//Copies a dentry found by filename into a passed pointer
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
//Copies a dentry accessed directly by index into a pointer
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
//Reading data by inode number helper.
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

//Reads data from a file; comes with tracking of where it left off.
int32_t read_file(int32_t fd, void* buf, int32_t nbytes);
//Writes into a file (or doesn't; our fs is limited)
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);
//Opens a file to read from when read is called; can open a file over a currently open one.
int32_t open_file(const uint8_t* filename);
//Closes the currently open (or no) file
int32_t close_file(int32_t fd);

//Reads directory names; comes with tracking of where it left off.
int32_t read_dir(int32_t fd, void* buf, int32_t nbytes);
//Writes into a directory (or doesn't; our fs is limited)
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes);
//Opens a directory
int32_t open_dir(const uint8_t* filename);
//Closes an (un)opened directory
int32_t close_dir(int32_t fd);

//Initializes our filesystem in memory
int32_t init_filesys_addr(uint32_t b_addr);


#endif


