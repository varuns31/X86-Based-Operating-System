#include "keyboard.h"

#define IRQ_LINE_KEYBOARD 0x01
#define KEYBOARD_PORT_ADDR 0x60

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

typedef struct keyboard_buffer {
    char *line;
    int length;
};

struct keyboard_buffer onscreen_buff[25]; 

int cur_line_counter;
char lines[25][80];

volatile char current_buffer[128];
volatile int curr_buff_length = 0;
volatile int prev_curr_buff_length = 0;
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
    int i = 0, j = 0;
    for(i = 0; i < 25; i++) {
        onscreen_buff[i].line = lines[i];
        onscreen_buff[i].length = 0;
        for(j = 0; j < 80; j++)
        {
            onscreen_buff[i].line[j]=' ';
        }
    }

    return; 
}

void scrolling()
{
    int y = get_screen_y();
    set_screen(0, y + 1);
    clear();
    int a, b;
    for(b = 1; b <= cur_line_counter; b++)
    {
        for(a = 0; a < onscreen_buff[b].length; a++)
        {
            onscreen_buff[b - 1].line[a] = onscreen_buff[b].line[a];
            putc(onscreen_buff[b].line[a]);
        }

        onscreen_buff[b-1].length = onscreen_buff[b].length;
        putc('\n');
    }

    onscreen_buff[cur_line_counter].length = 0;
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
        onscreen_buff[cur_line_counter].length--;

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
        putc('\n');
        // if(cur_line_counter==24)scrolling();
        // else
        cur_line_counter++;
    }

    // if(onscreen_buff[buf_line_counter].length==80 && cur_line_counter>=24)
    // {
    //     send_eoi(IRQ_LINE_KEYBOARD); 
    //     return;
    // }
    // write character to the screen
    if(curr_buff_length >= 126)
    {
        send_eoi(IRQ_LINE_KEYBOARD); 

        return;
    }

    onscreen_buff[cur_line_counter].line[onscreen_buff[cur_line_counter].length] = to_print;
    current_buffer[curr_buff_length] = to_print;
    
    curr_buff_length++;
    onscreen_buff[cur_line_counter].length += 1;

    //printf("%d\n",scan_code);
    putc(to_print);
    //printf("\n%c",to_print);

    send_eoi(IRQ_LINE_KEYBOARD); 

    return;
}

int32_t terminal_open (const uint8_t* filename) {
    clear();
    set_screen(0, 0);
    cur_line_counter = 0;
    curr_buff_length = 0;
    return 0;
}

int32_t terminal_close (int32_t fd) {
    if(fd == 0 || fd == 1) return -1;
    return 0;
}

int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes) {
    enter_pressed = 0;
    while(enter_pressed != 1) {}
    printf("Done\n");
    // memcpy(buf, current_buffer, 128);
    return 0;
}

int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes) {

    return 0;
}

