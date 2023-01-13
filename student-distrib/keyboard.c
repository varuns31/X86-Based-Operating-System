#include "keyboard.h"
#include "pcb.h"
#include "paging.h"
#include "pit.h"
#include "piano.h"

// given by the docs
#define IRQ_LINE_KEYBOARD 0x01
#define KEYBOARD_PORT_ADDR 0x60
#define VIDEO_START 0xB8000
#define NUM_ROWS_SCREEN 80
#define NUM_COLS_SCREEN 24
#define MAX_BUFFER_SIZE 128
#define HISTORY_NUM 100
extern int return_to_user_eip;

extern volatile int piano;

// create the keyboard map
char key_map[2][128] = {
    {
        'x', 'x', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 'x',  
        'x', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        'x', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 'x',
        'x', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 'x',
        'x', 'x' , ' '
    },
    {
        'x', 'x', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 'x',  
        'x', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        'x', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 'x',
        'x', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 'x',
        'x', 'x' , ' '
    }
};

struct keyboard_mapper {
    int caps_lock;
    int shift;
    int ctrl;
    int alt;
} key_tracker;

// size given by the docs
volatile char current_buffer[MAX_BUFFER_SIZE];
volatile char history[HISTORY_NUM][MAX_BUFFER_SIZE];
volatile int curr_buf_num = 0;
volatile int latest_buf_num = 0;
volatile int curr_buff_length = 0;
volatile int prev_curr_buff_length;
volatile int enter_pressed = 0;
volatile int vimexit = 0;
volatile int nochange = 0;

/* 
 *   save_and_restore_terminal_state
 *   DESCRIPTION: called when switching terminal to restore terminal
 *   INPUTS: switch_to_terminal - the terminal we are switching into
 *   OUTPUTS: none
 *   RETURN VALUE: 
 *   SIDE EFFECTS: change of terminal
 */
void save_and_restore_terminal_state(int switch_to_terminal) {
    // save curr terminal state
    terminals[cur_terminal].curr_buff_length = curr_buff_length;
    terminals[cur_terminal].prev_curr_buff_length = prev_curr_buff_length;
    terminals[cur_terminal].enter_pressed = enter_pressed;
    terminals[cur_terminal].curr_buf_num = curr_buf_num;
    terminals[cur_terminal].latest_buf_num = latest_buf_num;
    memcpy((void*) terminals[cur_terminal].history, (void*) history, MAX_BUFFER_SIZE * HISTORY_NUM);
    memcpy((void*) terminals[cur_terminal].buf, (void*) current_buffer, MAX_BUFFER_SIZE);

    // restore next terminal state
    set_screen(terminals[switch_to_terminal].screen_x, terminals[switch_to_terminal].screen_y);
    curr_buff_length =terminals[switch_to_terminal].curr_buff_length;
    prev_curr_buff_length = terminals[switch_to_terminal].prev_curr_buff_length;
    enter_pressed = terminals[switch_to_terminal].enter_pressed;
    curr_buf_num = terminals[switch_to_terminal].curr_buf_num;
    latest_buf_num = terminals[switch_to_terminal].latest_buf_num;
    memcpy((void*) history, (void*) terminals[switch_to_terminal].history, MAX_BUFFER_SIZE * HISTORY_NUM);
    memcpy((void*) current_buffer, (void*) terminals[switch_to_terminal].buf, MAX_BUFFER_SIZE);
    if(terminals[switch_to_terminal].og == 1)
    {
        ATTRIB = 0X0F;
    }
    else
    {
        ATTRIB = terminals[switch_to_terminal].attribute ; 
    }
    // set cursor
    set_cursor(terminals[switch_to_terminal].screen_x, terminals[switch_to_terminal].screen_y);
}

/* 
 *   is_alpha
 *   DESCRIPTION: Is the scan code an alphabet
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 1 for alphabet 0 for others
 *   SIDE EFFECTS: none
 */
int is_alpha(int scan_code) {
    //Values are bounds for alphabets on keyboard
    if( 
        (scan_code > 15 && scan_code < 26) || 
        (scan_code > 29 && scan_code < 39) ||
        (scan_code > 43 && scan_code < 51)
    ) return 1;
    
    return 0;
}

/* 
 * keyboard_handler_init
 *   DESCRIPTION: Handles the keyboard initialization
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void keyboard_handler_init() {
    // enable interrupts
    enable_irq(IRQ_LINE_KEYBOARD);
    
    key_tracker.caps_lock = 0;//caps initialised to not pressed
    key_tracker.shift = 0;//shift initialised to not pressed
    key_tracker.ctrl = 0;//ctrl initialised to not pressed
    key_tracker.alt = 0;//alt initialised to not pressed

    return; 
}
void vimkeyboardhandler(int scan_code)
{

    if(scan_code == 80) { //down
        int y;
        if(get_screen_y() < NUM_COLS_SCREEN)
        {
            y=get_screen_y()+1;
        }
        set_screen(get_screen_x(),y);
        set_cursor(get_screen_x(),y);
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }
    if(scan_code == 72) { //up
        int y;
        if(get_screen_y() > 0)
        {
            y=get_screen_y()-1;
        }
        set_screen(get_screen_x(),y);
        set_cursor(get_screen_x(),y);
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }   
        if(scan_code == 77) { //right
        int x;
        if(get_screen_x() < NUM_ROWS_SCREEN)
        {
            x=get_screen_x()+1;
        }
        set_screen(x,get_screen_y());
        set_cursor(x,get_screen_y());
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }   
        if(scan_code == 75) { //left
        int x;
        if(get_screen_x() > 0)
        {
            x=get_screen_x()-1;
        }
        set_screen(x,get_screen_y());
        set_cursor(x,get_screen_y());
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }   
    if(scan_code >= 62)
    {
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }
    // shift
    if(scan_code == 42 || scan_code == 54) {//shift opcode
        key_tracker.shift = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // capslock
    if(scan_code == 58) {//caps opcode
        key_tracker.caps_lock = (key_tracker.caps_lock) == 1 ? 0 : 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // alt l and r
    if(scan_code == 56) {//alt opcode
        key_tracker.alt = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // ctrl l and r
    if(scan_code == 29) {//ctrl opcode
        key_tracker.ctrl = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    if(key_tracker.ctrl && scan_code == 38) {//l opcode
        clear();
        set_screen(0, 0);
        set_cursor();
        send_eoi(IRQ_LINE_KEYBOARD);
        return; 
    }

    if(key_tracker.ctrl && scan_code == 37) {//k opcode
        vimexit = 0;
        send_eoi(IRQ_LINE_KEYBOARD);
        return; 
    }

    if(key_tracker.ctrl && scan_code == 36) {//j opcode
        vimexit = 0;
        nochange = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return; 
    }

    if(scan_code == 14) //backspace opcode
    {
        int x = get_screen_x();
        int y = get_screen_y();
        int linear = y * 80 + x;//80 is numrows
        linear--;
        
        if(linear <= 0) {
            send_eoi(IRQ_LINE_KEYBOARD);
            return;  
        }

        set_screen(linear % NUM_ROWS_SCREEN, linear / NUM_ROWS_SCREEN);
        putc_keyboard(' ');
        set_screen(linear % NUM_ROWS_SCREEN, linear / NUM_ROWS_SCREEN);

        send_eoi(IRQ_LINE_KEYBOARD);
       return;
    }

    if(scan_code == 28) {       //enter opcode 
        putc_keyboard('\n');
    }
    char to_print;
    int caps_idx = key_tracker.shift ^ key_tracker.caps_lock; //calculate what to print based on shifts and caps
    if(!is_alpha(scan_code)) caps_idx = key_tracker.shift;
    to_print = key_map[caps_idx][scan_code];
    putc_keyboard(to_print);

    send_eoi(IRQ_LINE_KEYBOARD); 

    return;
}
/* 
 * keyboard_handler
 *   DESCRIPTION: interrupt handler for keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clobbers ports for device
 */
void keyboard_handler() {
    cli();

    int scan_code = inb(KEYBOARD_PORT_ADDR);

    if(piano == 1) {
        piano_handler(scan_code);
        return;
    }
    
    //printf_keyboard("%d\n", scan_code);
    // 0x80 is the upper bound for preventing the key up scancode
    if(scan_code > 0x80) {

        if(scan_code == 170 || scan_code == 182) key_tracker.shift = 0;//Shift releases opcode
        if(scan_code == 184) key_tracker.alt = 0;//alt releases opcode
        if(scan_code == 157) key_tracker.ctrl = 0;//ctrl releases opcode
        
        // send eoi if scancode out of bounds
        send_eoi(IRQ_LINE_KEYBOARD);
        
        return;
    }

    if(vimexit == 1)
    {
        vimkeyboardhandler(scan_code);
        return;
    }


    if(scan_code == 80) { //down
        if(curr_buf_num + 1 > latest_buf_num) {
            // send eoi if scancode out of bounds
            send_eoi(IRQ_LINE_KEYBOARD);
            return;
        }
        int i=0;
        if(curr_buf_num + 1 == latest_buf_num) {
            // send eoi if scancode out of bounds
            curr_buf_num++;
            for(i=0 ;i < strlen(current_buffer) ;i++)
            {
            backspace();
            }
            curr_buff_length = 0;
            send_eoi(IRQ_LINE_KEYBOARD);
            return;
        }
        curr_buf_num++;
        for(i=0 ;i < strlen(current_buffer) ;i++)
        {
            backspace();
        }
        memcpy((void*) current_buffer, (void*) history[curr_buf_num], MAX_BUFFER_SIZE);
        curr_buff_length = strlen(history[curr_buf_num]);
        printf_keyboard(current_buffer);
    }

    if(scan_code == 72) { //up
        if(curr_buf_num - 1 < 0) {
            // send eoi if scancode out of bounds
            send_eoi(IRQ_LINE_KEYBOARD);
            return;
        }

        curr_buf_num--;
        int i=0;
        for(i=0 ;i < strlen(current_buffer) ;i++)
        {
            backspace();
        }
        memcpy((void*) current_buffer, (void*) history[curr_buf_num], MAX_BUFFER_SIZE);
        curr_buff_length = strlen(history[curr_buf_num]);
        printf_keyboard(current_buffer);
    }

    if(scan_code >= 62)
    {
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }
    // shift
    if(scan_code == 42 || scan_code == 54) {//shift opcode
        key_tracker.shift = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // capslock
    if(scan_code == 58) {//caps opcode
        key_tracker.caps_lock = (key_tracker.caps_lock) == 1 ? 0 : 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // alt l and r
    if(scan_code == 56) {//alt opcode
        key_tracker.alt = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // ctrl l and r
    if(scan_code == 29) {//ctrl opcode
        key_tracker.ctrl = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    if(key_tracker.ctrl && scan_code == 38) {//l opcode
        clear();
        set_screen(0, 0);
        set_cursor();
        curr_buff_length = 0;
        send_eoi(IRQ_LINE_KEYBOARD);
        return; 
    }

    if(key_tracker.ctrl && scan_code == 37) {//k opcode
        vimexit = 0;
        send_eoi(IRQ_LINE_KEYBOARD);
        return; 
    }

    if(scan_code == 14) //backspace opcode
    {
       backspace();
       return;
    }

    if(scan_code == 28) {       //enter opcode 
        prev_curr_buff_length = curr_buff_length;
        putc_keyboard('\n');
        terminals[cur_terminal].enter_pressed = 1;//marks enter pressed

        if(curr_buff_length == 0)
        {
            send_eoi(IRQ_LINE_KEYBOARD); 
            return;
        }

        if(latest_buf_num >= HISTORY_NUM){
            latest_buf_num--;
            char temp_buff[MAX_BUFFER_SIZE];
            memmove((void*) history, (void*) history[1], MAX_BUFFER_SIZE * (HISTORY_NUM - 1));
            memmove((void*) history[(HISTORY_NUM - 1)], (void*) temp_buff, MAX_BUFFER_SIZE);
        }

        memmove((void*) history[latest_buf_num], (void*) current_buffer, curr_buff_length);

        latest_buf_num++; 
        curr_buf_num = latest_buf_num;

        // putc_keyboard('\n');
        // puts_keyboard(history[0]);
        // putc_keyboard('\n');
        // puts_keyboard(history[1]);
        // putc_keyboard('\n');
        // puts_keyboard(history[2]);
        // putc_keyboard('\n');
        
        curr_buff_length = 0;
        send_eoi(IRQ_LINE_KEYBOARD); 
        return;
    }

    // function keys. 59 is f1
    if(scan_code >10 && scan_code < 14 ) {
        if(key_tracker.alt == 1)
        {
            // finding function key num
            if(cur_terminal == scan_code - 10){
                send_eoi(IRQ_LINE_KEYBOARD);
                return;
            }
            
            // function call for terminal switch
            switch_terminal(scan_code - 10); // finding function key num
            send_eoi(IRQ_LINE_KEYBOARD);
            return;

        } else{
            send_eoi(IRQ_LINE_KEYBOARD);
            return;
        }
        
    }

    char to_print;
    int caps_idx = key_tracker.shift ^ key_tracker.caps_lock; //calculate what to print based on shifts and caps
    if(!is_alpha(scan_code)) caps_idx = key_tracker.shift;
    to_print = key_map[caps_idx][scan_code];
    
    if(curr_buff_length > (MAX_BUFFER_SIZE - 2)){
        send_eoi(IRQ_LINE_KEYBOARD); 
        return;
    }

    current_buffer[curr_buff_length] = to_print;
    curr_buff_length++;

    putc_keyboard(to_print);

    send_eoi(IRQ_LINE_KEYBOARD); 

    return;
}

/* 
 * terminal_open
 *   DESCRIPTION: terminal open initialises terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE:  pass/fail
 *   SIDE EFFECTS: 
 */
int32_t terminal_open (const uint8_t* filename) {
    clear();
    set_screen(0, 0);
    curr_buff_length = 0;
    enter_pressed = 0;
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    int fd_idx;

    // check if fd is within range of 2 to 7 becuase stdin and stdout are not allowed
    for(fd_idx = 2; fd_idx < 8; fd_idx++){
        if(pcb->process_file_array[fd_idx].flags == 0){
            // set the file array at specified location
            puts((char *) filename);
            printf(" has been opened at position %d\n", fd_idx);
            pcb->process_file_array[fd_idx].inode_num = 0;
            pcb->process_file_array[fd_idx].file_pos = 0;
            pcb->process_file_array[fd_idx].flags = 1; // indicated in use
            return fd_idx;
        }
    }
    printf("Error: Too many files are open\n");
    return -1;
}

/* 
 * terminal_close
 *   DESCRIPTION: closes terminal 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE:  pass/fail
 *   SIDE EFFECTS: 
 */
int32_t terminal_close (int32_t fd) {
    if(fd == 0 || fd == 1) return -1;
    return 0;
}

/* 
 * terminal_read
 *   DESCRIPTION: reads keyboard buffer and passes to buf
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pass/fail
 *   SIDE EFFECTS: 
 */
int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes) {
    
    if(buf == NULL) return -1;
    curr_buff_length = 0;

    enter_pressed = 0;

    // end of line char
    char eol[2] = "\n\0";
    sti();
    while(terminals[pit_curr_terminal].enter_pressed != 1) {
    }
    // cli();
    terminals[pit_curr_terminal].enter_pressed = 0;
    char* buff = buf;
    int bytes_read = 0;

    if(nbytes > prev_curr_buff_length) {
        memcpy((void*)buf, (void*)current_buffer, prev_curr_buff_length);
        memcpy((void*)(buff + prev_curr_buff_length),(void*) eol, 2);
        bytes_read = prev_curr_buff_length + 1;
    } else {
        memcpy((void*)buf, (void*)current_buffer, nbytes);
        memcpy((void*)(buff + nbytes), (void*)eol, 2);
        bytes_read = nbytes + 1;
    }

    return bytes_read;
}

/* 
 * terminal_write
 *   DESCRIPTION: writes to screen from buffer
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pass/fail
 *   SIDE EFFECTS: 
 */
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes) {
    if(buf == NULL) return -1;
    int i;
    const char* buff = buf;
    for(i = 0; i < nbytes; i++) {
        putc(buff[i]);
    }
    return nbytes;
}

/* 
 * keyboard_open
 *   DESCRIPTION: opens keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pass/fail
 *   SIDE EFFECTS: 
 */
int32_t keyboard_open (const uint8_t* filename) {
    keyboard_handler_init();
    pcb_t* pcb = (pcb_t *) find_pcb(curr_pid);
    int fd_idx;

    // check if fd is within range of 2 to 7 becuase stdin and stdout are not allowed
    for(fd_idx = 2; fd_idx < 8; fd_idx++){
        if(pcb->process_file_array[fd_idx].flags == 0){
            pcb->process_file_array[fd_idx].inode_num = 0;
            pcb->process_file_array[fd_idx].file_pos = 0;
            pcb->process_file_array[fd_idx].flags = 1; // indicated in use
            return fd_idx;
        }
    }
    printf("Error: Too many files are open\n");
    return -1;
}

/* 
 * keyboard_close
 *   DESCRIPTION: closes keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pass/fail
 *   SIDE EFFECTS: 
 */
int32_t keyboard_close (int32_t fd) {
    // error if we try to close first 2 fds
    if(fd == 0 || fd == 1) return -1;
    return 0;
}
/* 
 * keyboard_read
 *   DESCRIPTION: reads keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pass/fail
 *   SIDE EFFECTS: 
 */
int32_t keyboard_read (int32_t fd, void* buf, int32_t nbytes) {
    
    if(buf == NULL) return -1;

    enter_pressed = 0;

    // end of line char
    char eol[2] = "\n\0";

    while(enter_pressed != 1) {}
    char* buff = buf;
    int bytes_read = 0;

    // nbytes is more than buffer length
    if(nbytes > prev_curr_buff_length) {
        memcpy((void*)buf, (void*)current_buffer, prev_curr_buff_length);
        memcpy((void*)(buff + prev_curr_buff_length), (void*)eol, 2);

        // adding 1 for end of line char
        bytes_read = prev_curr_buff_length + 1;
    } else {
        memcpy((void*)buf, (void*)current_buffer, nbytes);
        memcpy((void*)(buff + nbytes), (void*)eol, 2);

        // adding 1 for end of line char
        bytes_read = nbytes + 1;
    }

    return bytes_read;
}

/* 
 * keyboard_write
 *   DESCRIPTION: writes to keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: pass/fail
 *   SIDE EFFECTS: 
 */
int32_t keyboard_write (int32_t fd, const void* buf, int32_t nbytes) {
    return 0;
}

void backspace()
{
    if(curr_buff_length == 0)
        {
            send_eoi(IRQ_LINE_KEYBOARD);
            return;
        }
        int x = get_screen_x();
        int y = get_screen_y();
        int linear = y * 80 + x;//80 is numrows
        linear--;
        
        if(linear < 0 || (((linear + 1) % NUM_ROWS_SCREEN) == 0 && curr_buff_length == 0)) {
            send_eoi(IRQ_LINE_KEYBOARD);
            return;  
        }

        set_screen(linear % NUM_ROWS_SCREEN, linear / NUM_ROWS_SCREEN);
        putc_keyboard(' ');
        set_screen(linear % NUM_ROWS_SCREEN, linear / NUM_ROWS_SCREEN);

        curr_buff_length--;

        send_eoi(IRQ_LINE_KEYBOARD);
        return;
}
