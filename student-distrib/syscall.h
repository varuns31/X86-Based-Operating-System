#include "syscall_linkage.h"
#include "lib.h"
#include "pcb.h"
#include "rtc.h"
#include "keyboard.h"
#include "fs.h"
#include "paging.h"
#include "x86_desc.h"
#include "ret_to_user.h"
#include "pit.h"

int32_t system_halt (uint8_t status);
int32_t system_execute (const uint8_t* command);
int32_t system_read (int32_t fd, void* buf, int32_t nbytes);
int32_t system_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t system_open (const uint8_t* filename);
int32_t system_close (uint32_t fd);
int32_t system_getargs (uint8_t* buf, int32_t nbytes);
int32_t system_vidmap (uint8_t** screen_start);
int32_t system_set_handler (int32_t signum, void* handler_address);
int32_t system_sigreturn (void);

int32_t system_dummy (int32_t dummy);
int32_t ret_fail ();
void clean_buffer (char * buf);
extern int32_t get_first_inactive_pid();
extern void set_pcb_by_pid (pcb_t * cur_pcb, int pid);

extern int32_t terminal_open (const uint8_t* filename);
extern int32_t terminal_close (int32_t fd);
extern int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t keyboard_open (const uint8_t* filename);
extern int32_t keyboard_close (int32_t fd);
extern int32_t keyboard_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t keyboard_write (int32_t fd, const void* buf, int32_t nbytes);

extern int32_t fs_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t fs_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t fs_open (const uint8_t* filename);
extern int32_t fs_close (int32_t fd);

extern int32_t directory_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t directory_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t directory_open (const uint8_t* filename);
extern int32_t directory_close (int32_t fd);

extern int32_t rtc_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t rtc_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t rtc_open (const uint8_t* filename);
extern int32_t rtc_close (int32_t fd);

extern int32_t system_execute (const uint8_t* command);
extern int ATTRIB;

