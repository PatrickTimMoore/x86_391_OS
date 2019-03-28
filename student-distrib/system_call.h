#ifndef SYS_CALL_H
#define SYS_CALL_H

#include "types.h"

#define FILES_NUM	8

// typedef struct file_desc{
// 	uint32_t* file_ops_ptr;
// 	uint32_t inode_num;
// 	uint32_t file_pos;
// 	int32_t flags;
// } file_desc_t;

// typedef struct proc_ctrl_b{
// 	uint32_t pid;
// 	uint32_t esp0;
// 	uint32_t ss0;
// 	uint32_t esp;
// 	uint32_t ss;
// 	file_desc_t file_arr[FILES_NUM];
// 	uint32_t next_idx;
// }proc_ctrl_b_t;

// proc_ctrl_b_t pcb;


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
extern int32_t init_pcb();


#endif
