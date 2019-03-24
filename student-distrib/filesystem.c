#include "filesystem.h"
#include "types.h"
#include "lib.h"


/*
int32_t hasEOS(const int8_t* s1, int len);

//helper

// Function:
//  Description:
//  inputs: none
//  outputs: 0 for success and -1 for failure
//  effects:
//
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
*/

/* Function: int32_t init_filesys_addr
*  Description: Called in order to initialize the filesystem.
*  Inputs: b_addr - the address of the start of the boot block
*  Outputs: 0 for success and -1 for failure
*  Effects:
*       * Sets boot_block_ptr to the given address,
*       * Determines the start locations of the inode and data blocks
*       * Initializes structs for keeping track of opened files and dirs
*/
int32_t init_filesys_addr(uint32_t b_addr){
	boot_block_ptr = (boot_block_t*)b_addr;

	// printf("Boot block set! We have %d dentries,
	//  %d inodes, and %d data blocks.\n", 
	// 	boot_block_ptr->dentry_count, 
	// 	boot_block_ptr->inode_count, 
	// 	boot_block_ptr->datablock_count);

	// inodes are located at the first 4kb block after the boot block -LLY
	inodes = (inode_t*) (boot_block_ptr + 1);

    // data blocks begin after the Nth (N equal to inode_count) 4kb inode block --LLY
	datas = (data_block_t*) (inodes + (boot_block_ptr->inode_count));

    /* initialization of currfile and currdir, where no files
     * nor directories are initially open or read -LLY */
	currfile.open = 0;
	currfile.bytes_read = 0;

	currdir.open = 0;
	currdir.bytes_read = 0;

	return 0;
}


/* Function: int32_t read_dentry_by_name
*  Description: Finds directory entry by given filename.
*  Inputs:
*           * fname - name of file to find
*           * dentry_ret - dentry struct to fill with file info
*  Outputs:
*           * 0 for success and -1 for failure
*           * dentry struct filled out (is this an "output" or an effect?)
*  Effects: Changes dentry struct
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry_ret){
	int i;          // tracks current dentry idx; for loop ctr
	dentry_t d;     // holds current dentry

	// check for nullptr -LLY
	if(dentry_ret == NULL)
	    return -1;

	// check if fname given is even possible -LLY
	if(strlen((int8_t*) fname) > MAX_FNAME_LEN)
	    return -1;

	// NUMFILES searches all 63, dentry_count only what is required
	for(i = 0; i < boot_block_ptr->dentry_count; i++){
		d = boot_block_ptr->dentries[i];

        // we may run the risk of buffer overflow or overrun, must be double checked -LLY
        if (strncmp((int8_t *) fname, (int8_t *) d.file_name, MAX_FNAME_LEN) == 0){
			// we don't copy from (int8_t*)d.file_name as the source because it is not nul-terminated -LLY
			strncpy((int8_t *) dentry_ret->file_name, (int8_t *) fname, MAX_FNAME_LEN);
			dentry_ret->file_type = d.file_type;
			dentry_ret->inode_num = d.inode_num;

			return 0;
        }
	}
	printf("read_dentry_by_name failed.\n");
	return -1;
}

/* Function: int32_t read_dentry_by_index
*  Description: Finds directory entry by index.
*  Inputs:
*           * index - index of file to find
*           * dentry_ret - dentry struct to fill with file info
*  Outputs:
*           * 0 for success and -1 for failure
*           * dentry struct filled out (is this an "output" or an effect?)
*  Effects: Changes dentry struct
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry_ret){
	dentry_t d; // holds the found dentry at index

	// check for nullptr -LLY
	if (dentry_ret == NULL)
	    return -1;

	// check if proper index given -LLY
	if (index < 0 || index >= boot_block_ptr->dentry_count)
	    return -1;

    // get and copy dentry information
	d = boot_block_ptr->dentries[index];

	strncpy((int8_t*)dentry_ret->file_name, (int8_t*) d.file_name, MAX_FNAME_LEN);
	dentry_ret->file_type = d.file_type;
	dentry_ret->inode_num = d.inode_num;

	return 0;
}

/* Function: int32_t read_data
*  Description: Reads data from inode of a certain length from offset start point
*  Inputs:
*           * inode_idx - index node index of file to get data from
*           * offset - from where in the file to start reading
*           * buf - the buffer to fill                          (?? should we check this further?)
*           * length - the amount of data to read into buf      (?? is amount the correct word?)
*  Outputs:
*           * number of bytes read or -1 for failure
*  Effects: buf filled with requested data (?? should we clear buffer if failed?)
*/
int32_t read_data(uint32_t inode_idx, uint32_t offset, uint8_t* buf, uint32_t length){
    int i;          // keeps track of current byte in inode data block
    int data_id;    // indicates the data block to retrieve some data from
	inode_t ino;    // keeps track of inode of index inode_idx

    // check for nullptr -LLY
	if(buf == NULL)
	    return -1;

	// check if valid inode idx -LLY
	if(inode_idx < 0 || inode_idx >= boot_block_ptr->inode_count){
		printf("read_data: Out of bounds at %d with limit %d\n", inode_idx, boot_block_ptr->inode_count);
		return -1;
	}

	// Get our inode
	ino = inodes[inode_idx];

    // check if offset is valid
	if(offset > ino.len_b){
		printf("read_data: Out of bounds at %d offset, limit is %d len\n", offset, ino.len_b);
		return -1;
	}

	for(i = offset; i < offset+length && i < ino.len_b; i++){
		// (i/BLOCK_SIZE) <--- Which 4KB block?
        data_id = ino.data_ids[i/BLOCK_SIZE];
        // check to see if valid data_blocks are being encountered,
        //      datablocks being between [0, datablock_count)               -LLY
		if(data_id < 0 || data_id >= boot_block_ptr->datablock_count ){
		    // might want to clear buf if security was of utmost importance here, with a function -LLY
			printf("read_data: invalid data block index found\n");
			return -1;
		}

		// (i%BLOCK_SIZE) <--- Which byte of the 4KB?
		buf[i - offset] = datas[data_id].data[i%BLOCK_SIZE];
		//^ take a second on this line
	}

    // return total number of bytes read into buf
	return i - offset;
}


/* Function: int32_t read_file
*  Description:
*  Inputs:
*           * fd - currently unused                 (!!)
*           * buf - the buffer to return the read data in
*           * nbytes - number of bytes to put into buf
*  Outputs:
*           * Number of bytes read;
*           * 0 means we've reached the end (or 0 bytes ordered);
*           * -1 means error
*  Effects:
*           * Fills buf with data
*           * Changes currfile struct
*/
int32_t read_file(int32_t fd, void* buf, int32_t nbytes){
	int ret;

	if(!currfile.open){
		printf("%s\n", "read_file ERROR: No file currently open!\n");
		return -1;
	}
	if(buf == NULL){
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
	else
	    currfile.bytes_read += ret;

	return ret;
}

/* Function: int32_t write_file
*  Description: Nothing, we currently have a read-only filesystem
*  Inputs:
*           * fd - File descriptor of file we would write to
*           * buf - buffer that would contain some info to write
*           * nbytes - the number of bytes we would write to the file
*  Outputs: -1 for failure
*  Effects: Nothing
*/
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

/* Function: int32_t open_file
*  Description: Attempts to open file
*  Inputs: filename - the filename of the file to open
*  Outputs: 0 for success and -1 for failure
*  Effects: Changes currfile struct
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


// !!!!!
/* Function: int32_t close_file
*  Description: Closes file denoted by fd.
*  Inputs: fd - currently nothing...                        (!!)
*  Outputs: 0 for success and -1 for failure
*  Effects: Changes currfile
*  Issues: Not correct. this should check if:
*           1. the file descriptor is valid
*           2. if the file is open
*           3. if the file is being read
*/
int32_t close_file(int32_t fd){
	currfile.open = 0;
	currfile.bytes_read = 0;
	return 0;
}


/* Function: int32_t read_dir
*  Description: Reads from directory.
*  Inputs:
*           * fd - the directory's file descriptor
*           * buf - the buffer to return the data in
*           * nbytes - the amount of data to read and size of buf
*  Outputs: Either number of bytes read or -1 for failure
*  Effects:
*           * Changes buf
*           * Changes currdir
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
	if(buf == NULL){
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


/* Function: int32_t write_dir
*  Description: Nothing, you can't write to a directory (??)
*  Inputs: Unused
*  Outputs: -1 for failure
*  Effects: Nothing
*/
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}


/* Function: int32_t open_dir
*  Description: Opens directory for reading
*  Inputs: filename - name of directory to open
*  Outputs: 0 for success and -1 for failure
*  Effects: Changes currdir
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


/* Function: int32_t close_dir
*  Description: Closes open directory
*  Inputs: Unused fd, we only have one directory in a flat fs
*  Outputs: 0 for success (and -1 for failure eventually)
*  Effects: Changes currdir
*  Issues: Should check if the directory is actually open no?
*/
int32_t close_dir(int32_t fd){
	currdir.open = 0;
	currdir.bytes_read = 0;
	currdir.curr_idx = 0;
	return 0;
}


