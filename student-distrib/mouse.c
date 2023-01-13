#include "mouse.h"
#include "keyboard.h"

uint8_t mouse_cycle = 0;
int8_t mouse_data[3];

extern volatile int vimexit;

uint8_t mouse_current_state[MOUSE_STATE_NUM];
uint8_t mouse_prev_state[MOUSE_STATE_NUM];

void mouse_handler() {

    switch (mouse_cycle) {
        case 0: {
            mouse_data[0] = mouse_read();

            if((mouse_data[0] & 0x8) != 0x8) {
                break;
            }

            mouse_cycle++;
            break;
        }

        case 1: {
            mouse_data[1] = mouse_read();
            mouse_cycle++;
            break;
        }

        case 2: {
            mouse_data[2] = mouse_read();
            mouse_x += mouse_data[1] / 10;
            mouse_y -= mouse_data[2] / 10;

            if(mouse_x < 0) mouse_x = 0;
            if(mouse_y < 0) mouse_y = 0;
            if(mouse_x > 79) mouse_x = 79;
            if(mouse_y > 24) mouse_y = 24;

            if((mouse_data[0] & 0x1) == 0x1 && vimexit == 1) {
                set_screen(mouse_x, mouse_y);
                mouse_current_state[0] = 1;
            }

            set_cursor_mouse(mouse_x, mouse_y);
            mouse_cycle = 0;
            break;
        }

        default: {
            mouse_cycle = 0;
            break;
        }

    }

    // mouse_event_dedect();
    memcpy(mouse_prev_state, mouse_current_state, MOUSE_STATE_NUM);
    send_eoi(MOUSE_IRQ);
}

/**
 * Init mouse
 */
void init_mouse() {
    memset(mouse_current_state, 0, MOUSE_STATE_NUM);
    memset(mouse_prev_state, 0, MOUSE_STATE_NUM);
    mouse_x = 50;
    mouse_y = 50;
    uint8_t status;

    mouse_wait(MOUSE_WAIT_TYPE_SIGNAL);
    outb(0xA8, MOUSE_CTRL_PORT);

    mouse_wait(MOUSE_WAIT_TYPE_SIGNAL);
    outb(0x20, MOUSE_CTRL_PORT);
    mouse_wait(MOUSE_WAIT_TYPE_DATA);
    status = (inb(MOUSE_DATA_PORT) | 2);
    mouse_wait(MOUSE_WAIT_TYPE_SIGNAL);
    outb(0x60, MOUSE_CTRL_PORT);
    mouse_wait(MOUSE_WAIT_TYPE_SIGNAL);
    outb(status, MOUSE_DATA_PORT);

    /* Use the default settings */
    mouse_write(0xF6);
    mouse_read();  /* Wait for ack */

    /* Enable the mouse */
    mouse_write(0xF4);
    mouse_read();  /* Wait for ack */

    enable_irq(MOUSE_IRQ);
}

/**
 * Write some data to mouse
 * @param data
 */
void mouse_write(uint8_t data) {
    mouse_wait(MOUSE_WAIT_TYPE_SIGNAL);
    outb(0xD4, MOUSE_CTRL_PORT); /* We are sending a command */
    mouse_wait(MOUSE_WAIT_TYPE_SIGNAL); /* Wait for ack */
    outb(data, MOUSE_DATA_PORT);
}

/**
 * Read data from mouse
 * @return
 */
uint8_t mouse_read() {
    mouse_wait(MOUSE_WAIT_TYPE_DATA);
    return inb(MOUSE_DATA_PORT);
}

/**
 * Wait for some circle
 * @param type
 */
void mouse_wait(uint8_t type) {
    uint32_t timeout = MOUSE_TIMEOUT;
    switch (type) {
        case MOUSE_WAIT_TYPE_DATA: {
            /* Wait until we can read */
            while(timeout--) {
                if((inb(MOUSE_CTRL_PORT) & 1) == 1) return;
            }
            break;
        }
        
        case MOUSE_WAIT_TYPE_SIGNAL: {
            while(timeout--) {
                if((inb(MOUSE_CTRL_PORT) & 2) == 0) return;
            }

            break;
        }

        default: return;
    }
}
