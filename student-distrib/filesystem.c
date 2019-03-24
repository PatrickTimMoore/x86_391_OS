#include "filesystem.h"
#include "types.h"
#include "lib.h"


int32_t hasEOS(const int8_t* s1, int len);

//helper

/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t hasEOS(const int8_t* s1, int len) {
    int32_t i;
    for (i = 0; i < len; ++i)
    {
    	if(s1[i] == '\0'){
    		return i;
    	}
    }
    return -1;
}


/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t init_filesys_addr(uint32_t b_addr){
	boot_block_ptr = (boot_block_t*)b_addr;

	// printf("Boot block set! We have %d dentries,
	//  %d inodes, and %d data blocks.\n", 
	// 	boot_block_ptr->dentry_count, 
	// 	boot_block_ptr->inode_count, 
	// 	boot_block_ptr->datablock_count);


	//512*8 = 4096 hence 4KB worth of addresses
	// inodes = (inode_t*)(((uint32_t)b_addr) + 4096);
	// inodes = (inode_t*) (((uint32_t)b_addr) + sizeof(boot_block_t));
	inodes = (inode_t*) (boot_block_ptr + 1);

	// datas = (data_block_t*)(((uint32_t)inodes) + ((boot_block_ptr->inode_count)*4096));
	datas = (data_block_t*) (inodes + (boot_block_ptr->inode_count));

	currfile.open = 0;
	currfile.bytes_read = 0;

	currdir.open = 0;
	currdir.bytes_read = 0;

	return 0;
}


/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
	// printf("Reminder: We have %d dentries, %d inodes, 
	// 	and %d data blocks.\n", 
	// 	boot_block_ptr->dentry_count, 
	// 	boot_block_ptr->inode_count, 
	// 	boot_block_ptr->datablock_count);

	//were we passed a nullptr?
	if(!dentry){return -1;}

	//is our name even valid?
	if(strlen((int8_t*)fname) > 32){return -1;}

	// printf("read_dentry_by_name! Initializing variables...\n");

	int i, len;
	dentry_t d;

	// printf("read_dentry_by_name (%s)! Entering loop...\n", fname);

	// NUMFILES searches all 63, dentry_count only what is required
	for(i = 0; i < boot_block_ptr->dentry_count; i++){
		d = boot_block_ptr->dentries[i];

		// Pick the correct amount of characters to compare because
		// of 1 goddamn EoS case
		// Like srsly just limit all names to 31 wtf saves computation
		if(hasEOS((int8_t*)d.file_name, 32) == -1){
			len = 32;
		}
		else if(strlen((int8_t*)d.file_name) != strlen((int8_t*)fname)){
			continue;
		}
		else{
			len = strlen((int8_t*)fname);
		}

		if(!strncmp((int8_t*)fname, (int8_t*)d.file_name, len)){
			// printf("read_dentry_by_name MATCH!!! memcpy...\n");
			//get exactly as many bytes as needed
			strncpy((int8_t*)dentry->file_name, (int8_t*)d.file_name, 32);
			dentry->file_type = d.file_type;
			dentry->inode_num = d.inode_num;

			return 0;
		}
	}
	printf("read_dentry_by_name failed.\n");
	return -1;
}

/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
	//were we passed a nullptr?
	if(!dentry){return -1;}

	//Check for out of bounds!
	if(index < 0 || index >= boot_block_ptr->dentry_count){return -1;}

	// printf("read_dentry_by_index! Initializing variables...\n");
	// int i;
	dentry_t d;

	d = boot_block_ptr->dentries[index];
	strncpy((int8_t*)dentry->file_name, (int8_t*)d.file_name, 32);
	dentry->file_type = d.file_type;
	dentry->inode_num = d.inode_num;

	return 0;
}

/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	inode_t ino;
	int i;
	if(inode < 0 || inode >= boot_block_ptr->inode_count){
		printf("read_data: Out of bounds at %d with limit %d\n", inode, boot_block_ptr->inode_count);
		return -1;
	}

	if(!buf){return -1;}

	//Get our inode
	ino = inodes[inode];
	// printf("read_data: We have %d possible bytes\n", ino.len_b);

	if(offset > ino.len_b){
		printf("%s (we have %d)\n", "read_data: we don't have that many bytes go away\n", ino.len_b);
		return -1;
	}

	for(i = offset; i < offset+length && i < ino.len_b; i++){
		//i/4096 <--- Which 4KB block?
		//i%4096 <--- Which byte of the 4KB?

		if(ino.data_ids[i/4096] < 0 || ino.data_ids[i/4096] >= boot_block_ptr->datablock_count ){
			printf("read_data: invalid data block index found\n");
			return -1;
		}

		buf[i-offset] = datas[ino.data_ids[i/4096]].data[i%4096];
		//^ take a second on this line
	}

	return i-offset;
}

/* Function:
*  Description:
*  inputs: none
*  outputs: Number of bytes read; 0 means we've reached the end (or 0 bytes ordered); -1 means error
*  effects:
*/
int32_t read_file(int32_t fd, void* buf, int32_t nbytes){
	int ret;

	if(!currfile.open){
		printf("%s\n", "read_file ERROR: No file currently open!\n");
		return -1;
	}
	if(!buf){
		printf("%s\n", "read_file ERROR: Buffer passed was NULL!\n");
		return -1;
	}
	if(nbytes < 0){
		printf("%s\n", "read_file ERROR: Requested negative bytes\n");
		return -1;
	}

	ret = read_data(currfile.dentry.inode_num, currfile.bytes_read, (uint8_t*)buf, nbytes);
	// printf("ret got %d\n", ret );
	if(ret == -1){
		printf("%s\n", "read_file ERROR: Read failed\n");
	}
	currfile.bytes_read += ret;

	return ret;
}

/* Function: int32_t write(int32_t fd, const void* buf, int32_t nbytes)
*  Description: Nothing but report a failure! This is read-only filesystem
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t open_file(const uint8_t* filename){
	if(		!read_dentry_by_name(filename, &(currfile.dentry))	){
		if(currfile.dentry.file_type != 2){
			printf("open_file ERROR: open_file failed (Not a file). Closing file...\n");
			currfile.open = 0;
			currfile.bytes_read = 0;
			return -1;
		}
		currfile.open = 1;
		currfile.bytes_read = 0;
		return 0;
	}
		
	printf("%s\n", "open_file ERROR: open_file failed (file not found)\n");
	return -1;
	
}

/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t close_file(int32_t fd){
	currfile.open = 0;
	currfile.bytes_read = 0;
	return 0;
}


/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t read_dir(int32_t fd, void* buf, int32_t nbytes){
	int i; //, ret;
	if(nbytes == 0){
		return 0;
	}
	if(!currdir.open){
		printf("%s\n", "read_dir ERROR: No dir currently open!\n");
		return -1;
	}
	if(!buf){
		printf("%s\n", "read_dir ERROR: Buffer passed was NULL!\n");
		return -1;
	}
	if(nbytes < 0){
		printf("%s\n", "read_dir ERROR: Requested negative bytes\n");
		return -1;
	}

	// printf("read_dir: we're valid\n");
	dentry_t d;
	if(!read_dentry_by_index(currdir.curr_idx, &d)){ //assuming if this is unsuccessful we're out of files

		// printf("read_dir: Got a dentry, copying file_name-------\n");
		//accomodate for 32 big
		uint8_t buf33[33];
		// buf33[32] = '\n';
		buf33[32] = '\0';
		for(i = 0; i < 32; i++){
			buf33[i] = d.file_name[i];
		}
		// printf("read_dir: Our og name was %s-------\n", d.file_name);

		// printf("read_dir: Our file_name we parsed is %s--------------------\n", buf33);
		// printf("read_dir: Copying name into buffer...\n");

		//Now we have a name to copy over
		//Make sure to get the \0 also if requested in nbytes
		for (i = 0; i < nbytes && i < strlen((int8_t*)buf33)+1; i++){
			// printf("read_dir: Copying byte %d....\n", i);
			((uint8_t*)buf)[i] = buf33[i];
		}

		// if(i == 32 && i < nbytes){
			// ((uint8_t*)buf)[i] = '\0';
		// }

		// printf("read_dir: Done, moving up file%d....\n", i);
		//No matter how much we copy, move on to next file for next read
		currdir.curr_idx++;
		currdir.bytes_read = 0;
		return i;
	}
	else{
		// printf("read_dir: Either done, or error in reading by index\n");
		//If we can't access anymore we assume we have no more files
		return 0;
	}


	// return -1;
}


/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}


/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t open_dir(const uint8_t* filename){
	if(		!read_dentry_by_name(filename, &(currdir.dentry))	){
		if(currdir.dentry.file_type != 1){
			printf("open_dir ERROR: open_dir failed (Not a dir). Closing dir...\n" );
			currdir.open = 0;
			currdir.bytes_read = 0;
			currdir.curr_idx = 0;
			return -1;
		}
		currdir.open = 1;
		currdir.bytes_read = 0;
		currdir.curr_idx = 0;
		return 0;
	}

	printf("%s\n", "open_dir ERROR: open_dir failed (dir not found)\n");
	return -1;
}


/* Function:
*  Description:
*  inputs: none
*  outputs: 0 for success and -1 for failure
*  effects:
*/
int32_t close_dir(int32_t fd){
	currdir.open = 0;
	currdir.bytes_read = 0;
	currdir.curr_idx = 0;
	return 0;
}


