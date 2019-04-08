#ifndef SYS_CALL_H
#define SYS_CALL_H

#include "types.h"

#define FILES_NUM		8
#define FUN_PTR_NUM		3
#define ARG_BUF_SIZE	200

//yeezy just jumped over this
typedef struct jump_man{
	int32_t (*open)(const uint8_t*);
	int32_t (*read)(int32_t, void*, int32_t);
	int32_t (*write)(int32_t, const void*, int32_t);
	int32_t (*close)(int32_t fd);
}jump_table_t;

typedef struct file_desc{
	jump_table_t *file_ops_ptr;
	int32_t inode_num;
	uint32_t file_pos;
	int32_t flags;
} file_desc_t;

typedef struct proc_ctrl_b{
	int32_t pid0;
	int32_t pid;
	uint32_t ebp0;
	uint32_t esp0;
	file_desc_t file_arr[FILES_NUM];
	uint8_t argbuf[ARG_BUF_SIZE];
}pcb_t;

// FD flags
// bit 0 = used
// bit 1 = read
// bit 2 = write

extern int32_t process_bit_map[6];

//yeet
extern int32_t halt (uint8_t status);
extern int32_t execute (const uint8_t* command);
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open (const uint8_t* filename);
extern int32_t close (int32_t fd);
extern int32_t getargs (uint8_t* buf, int32_t nbytes);
extern int32_t vidmap (uint8_t** screen_start);
extern int32_t set_handler (int32_t signum, void* handler_address);
extern int32_t sigreturn (void);


extern pcb_t* get_curr_pcb();
extern int32_t get_curr_pid();
extern pcb_t* get_pcb(int gpid);


#endif
