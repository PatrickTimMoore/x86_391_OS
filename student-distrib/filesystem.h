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
#define BOOT_BLOCK_ADDR 0x0040F000
#define INODE_ADDR		BOOT_BLOCK_ADDR + FOURKB_BYTES
#define BYTES_IN_4KB	4096

typedef struct dentry{
	uint8_t file_name[FNAME_LEN];
	uint32_t file_type;
	uint32_t inode_num;
	uint8_t res[DENTRY_RES];
} dentry_t;

typedef struct boot_block{
	uint32_t dentry_count;
	uint32_t inode_count;
	uint32_t datablock_count;
	uint8_t reserved[BOOT_BLOCK_RES];
	dentry_t dentries[NUMFILES];
} boot_block_t;

typedef struct inode{
	uint32_t len_b;
	uint32_t data_ids[MAX_DBLOCKS];
} inode_t;

typedef struct data_block{
	uint8_t data[BYTES_IN_4KB];
} data_block_t;


typedef struct curr_file{
	uint8_t open;
	uint32_t bytes_read;
	dentry_t dentry;
}curr_file_t;

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

//Our bookkeeping data structure
curr_file_t currfile;
curr_dir_t currdir;



// typedef union file_block{
// 	boot_block_t boot_block;
// 	inode_t inode;
// 	uint32_t data_block			:FOURKB_BITS;
// } file_block_t;

int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t read_file(int32_t fd, void* buf, int32_t nbytes);
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes);
int32_t open_file(const uint8_t* filename);
int32_t close_file(int32_t fd);

int32_t read_dir(int32_t fd, void* buf, int32_t nbytes);
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes);
int32_t open_dir(const uint8_t* filename);
int32_t close_dir(int32_t fd);

int32_t init_filesys_addr(uint32_t b_addr);


#endif


