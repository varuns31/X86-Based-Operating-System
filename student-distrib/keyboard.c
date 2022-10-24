#include "keyboard.h"

#define IRQ_LINE_KEYBOARD 0x01
#define KEYBOARD_PORT_ADDR 0x60
#define VIDEO_START 0xB8000

// create the keyboard map
char key_map[2][128] = {
    {
        'x', 'x', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 'x',  
        'x', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        'x', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 'x',
        'x', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 'x'
    },
    {
        'x', 'x', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 'x',  
        'x', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        'x', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 'x',
        'x', '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 'x'
    }
};

struct keyboard_mapper {
    int caps_lock;
    int shift;
    int ctrl;
    int alt;
} key_tracker;

int cur_line_counter;

volatile char current_buffer[128];
volatile int curr_buff_length = 0;
volatile int prev_curr_buff_length;
volatile enter_pressed = 0;

int is_alpha(int scan_code);

int is_alpha(int scan_code) {
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
    
    key_tracker.caps_lock = 0;
    key_tracker.shift = 0;
    key_tracker.ctrl = 0;
    key_tracker.alt = 0;

    cur_line_counter = get_screen_y();
    return; 
}

void scrolling() {
    int y = get_screen_y();
    
    char* video_mem = (char *)VIDEO_START;
    memmove(video_mem, video_mem + ((80 * 1 + 0) << 1), 160 * 24);
    int32_t i;
    for (i = 0; i < 80; i++) {
        *(uint8_t *)(video_mem + ((24 * 80 + i) << 1)) = ' ';
        *(uint8_t *)(video_mem + ((24 * 80 + i) << 1) + 1) = 0x3;
    }
    
    set_screen(0, y);
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
    int scan_code = inb(KEYBOARD_PORT_ADDR);
    
    // 0x80 is the upper bound for preventing the key up scancode
    if(scan_code > 0x80) {

        if(scan_code == 170 || scan_code == 182) key_tracker.shift = 0;
        if(scan_code == 184) key_tracker.alt = 0;
        if(scan_code == 157) key_tracker.ctrl = 0;
        
        // send eoi if scancode out of bounds
        send_eoi(IRQ_LINE_KEYBOARD);
        
        return;
    }

    // shift
    if(scan_code == 42 || scan_code == 54) {
        key_tracker.shift = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // capslock
    if(scan_code == 58) {
        key_tracker.caps_lock = (key_tracker.caps_lock) == 1 ? 0 : 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // alt l and r
    if(scan_code == 56) {
        key_tracker.alt = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    // ctrl l and r
    if(scan_code == 29) {
        key_tracker.ctrl = 1;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    if(key_tracker.ctrl && scan_code == 38) {
        clear();
        set_screen(0, 0);
        cur_line_counter = 0;
        curr_buff_length = 0;
        send_eoi(IRQ_LINE_KEYBOARD);
        return; 
    }

    if(scan_code == 14) 
    {
        int x = get_screen_x();
        int y = get_screen_y();
        int linear = y * 80 + x;
        linear--;
        
        if(linear < 0 || (((linear + 1) % 80) == 0 && curr_buff_length < 80)) {
            send_eoi(IRQ_LINE_KEYBOARD);
            return;  
        }

        if(x == 0) cur_line_counter--;

        set_screen(linear % 80, linear / 80);
        putc(' ');
        set_screen(linear % 80, linear / 80);

        curr_buff_length--;
        // onscreen_buff[cur_line_counter].length--;

        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    if(scan_code == 28) {        
        if(cur_line_counter >= 24) {
            scrolling();
            prev_curr_buff_length = curr_buff_length;
            curr_buff_length = 0;
            enter_pressed = 1;
            send_eoi(IRQ_LINE_KEYBOARD); 
            return;
        }
        else
        {
            prev_curr_buff_length = curr_buff_length;
            cur_line_counter++;
            curr_buff_length = 0;
            putc('\n');
            enter_pressed = 1;
            send_eoi(IRQ_LINE_KEYBOARD); 
            return;
        }
    }

    char to_print;
    int caps_idx = key_tracker.shift ^ key_tracker.caps_lock;
    if(!is_alpha(scan_code)) caps_idx = key_tracker.shift;
    to_print = key_map[caps_idx][scan_code];

    if (curr_buff_length == 80)
    {
        if(cur_line_counter>=24)
        {
            scrolling();
        }
        else
        {
            putc('\n');
            cur_line_counter++;
        }
    }


    if(curr_buff_length > 126)
    {
        send_eoi(IRQ_LINE_KEYBOARD); 

        return;
    }

    current_buffer[curr_buff_length] = to_print;
    curr_buff_length++;

    putc(to_print);

    send_eoi(IRQ_LINE_KEYBOARD); 

    return;
}

int32_t terminal_open (const uint8_t* filename) {
    clear();
    set_screen(0, 0);
    cur_line_counter = 0;
    curr_buff_length = 0;
    enter_pressed = 0;
    return 0;
}

int32_t terminal_close (int32_t fd) {
    if(fd == 0 || fd == 1) return -1;
    return 0;
}

int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes) {
    
    if(buf == NULL) return -1;

    enter_pressed = 0;
    char eol[2] = "\n\0";

    while(enter_pressed != 1) {}
    char* buff = buf;
    int bytes_read = 0;

    if(nbytes > prev_curr_buff_length) {
        memcpy(buf, current_buffer, prev_curr_buff_length);
        memcpy(buff + prev_curr_buff_length, eol, 2);
        bytes_read = prev_curr_buff_length + 1;
    } else {
        memcpy(buf, current_buffer, nbytes);
        memcpy(buff + nbytes, eol, 2);
        bytes_read = nbytes + 1;
    }

    return bytes_read;
}

int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes) {
    if(buf == NULL) return -1;
    int i;
    char* buff = buf;
    for(i = 0; i < nbytes; i++) {
        if(i > 80 && get_screen_y() >= 24) {
            scrolling();
        } else if(i % 80 == 0 && i != 0) {
            putc('\n');
            cur_line_counter++;
        }

        if(buff[i] == "\0") continue;
        putc(buff[i]);
    }
    return nbytes;
}

int32_t keyboard_open (const uint8_t* filename) {
    keyboard_handler_init();
    return 0;
}

int32_t keyboard_close (int32_t fd) {
    if(fd == 0 || fd == 1) return -1;
    return 0;
}

int32_t keyboard_read (int32_t fd, void* buf, int32_t nbytes) {
    
    if(buf == NULL) return -1;

    enter_pressed = 0;
    char eol[2] = "\n\0";

    while(enter_pressed != 1) {}
    char* buff = buf;
    int bytes_read = 0;

    if(nbytes > prev_curr_buff_length) {
        memcpy(buf, current_buffer, prev_curr_buff_length);
        memcpy(buff + prev_curr_buff_length, eol, 2);
        bytes_read = prev_curr_buff_length + 1;
    } else {
        memcpy(buf, current_buffer, nbytes);
        memcpy(buff + nbytes, eol, 2);
        bytes_read = nbytes + 1;
    }

    return bytes_read;
}

int32_t keyboard_write (int32_t fd, const void* buf, int32_t nbytes) {
    return 0;
}

