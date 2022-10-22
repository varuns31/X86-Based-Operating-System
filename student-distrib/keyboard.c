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

typedef struct keyboard_buffer{
    char *line;
    int length;
};

struct keyboard_buffer onscreen_buff[25]; 
int buf_length;

int screen_filled=0;
int cur_line_counter = 0;
char lines[25][128];

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
    buf_length=0;
    int i=0;
    for(i=0;i<25;i++)
    {
        onscreen_buff[i].line=lines[i];
        onscreen_buff[i].length=0;
    }
 
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
        cur_line_counter=0;
        buf_length=0;
        screen_filled=0;
        send_eoi(IRQ_LINE_KEYBOARD);
        return; 
    }
    if(scan_code == 14) 
    {
        int x = get_screen_x();
        int y = get_screen_y();
        int linear = y * 80 + x;
        linear--;
        
        if(linear < 0 || (((linear+1) % 80) ==0 && onscreen_buff[buf_length].length<80)) {
            send_eoi(IRQ_LINE_KEYBOARD);
            return;  
        }

        set_screen(linear % 80, linear / 80);
        putc(' ');
        set_screen(linear % 80, linear / 80);
        onscreen_buff[buf_length].length--;
        send_eoi(IRQ_LINE_KEYBOARD);
        return;
    }

    if(scan_code == 28) {
        cur_line_counter++;
        
        
        if(cur_line_counter >= 25) {
            // int x = get_screen_x();
            int y = get_screen_y();
            if(onscreen_buff[0].length>80)y--;
            clear();
            // int linear = y * 80 + x;
            // // cur_line_counter--;
            // set_screen(0,linear/80-1);
            int a,b;
            // if(onscreen_buff[0].length>80)
            // {
            //     cur_line_counter--;
            //     set_screen(0,y-1);
            // }
            // else 
            // {
            //     cur_line_counter-=2;
            //     set_screen(0,y-2);
            // }
            // screen_filled=1;
            for(b=0;b<buf_length;b++)
            {
                onscreen_buff[b-1].length=onscreen_buff[b].length;
                for(a=0;a<onscreen_buff[b].length;a++)
                {
                    if(a==80)putc('\n');
                    printf("%s\n",(onscreen_buff[b].line));
                    onscreen_buff[b-1].line[a]=onscreen_buff[b].line[a];
                }
            }
            onscreen_buff[buf_length].length=0;
            set_screen(0,y-1);
            // // putc('\n');
            // // set_screen(0,24);
        }
        else
        {
            buf_length++;
            if(buf_length==25)buf_length=0;
        }
    }

    char to_print;
    int caps_idx = key_tracker.shift ^ key_tracker.caps_lock;
    if(!is_alpha(scan_code)) caps_idx = key_tracker.shift;
    to_print = key_map[caps_idx][scan_code];

    if (onscreen_buff[buf_length].length==80)
    {
        putc('\n');
        cur_line_counter++;
    }

    // write character to the screen
     if(onscreen_buff[buf_length].length<128)
    {
        if(scan_code!=28)
        {
            onscreen_buff[buf_length].line[onscreen_buff[buf_length].length]=to_print;
            onscreen_buff[buf_length].length+=1;
        }
        //printf("%d\n",scan_code);
        putc(to_print);
        //printf("\n%c",to_print);
    }
    send_eoi(IRQ_LINE_KEYBOARD); 

    return;
}
