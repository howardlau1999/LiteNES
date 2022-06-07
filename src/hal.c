/*
This file presents all abstractions needed to port LiteNES.
  (The current working implementation uses allegro library.)

To port this project, replace the following functions by your own:
1) nes_hal_init()
    Do essential initialization work, including starting a FPS HZ timer.

2) nes_set_bg_color(c)
    Set the back ground color to be the NES internal color code c.

3) nes_flush_buf(*buf)
    Flush the entire pixel buf's data to frame buffer.

4) nes_flip_display()
    Fill the screen with previously set background color, and
    display all contents in the frame buffer.

5) wait_for_frame()
    Implement it to make the following code is executed FPS times a second:
        while (1) {
            wait_for_frame();
            do_something();
        }

6) int nes_key_state(int b) 
    Query button b's state (1 to be pressed, otherwise 0).
    The correspondence of b and the buttons:
      0 - Power
      1 - A
      2 - B
      3 - SELECT
      4 - START
      5 - UP
      6 - DOWN
      7 - LEFT
      8 - RIGHT
*/
#include "hal.h"
#include "fce.h"
#include "common.h"
#include "mmio.h"

#define REFRESH_TIMER_LIMIT 2083333
volatile int timer_fired = 0;

#define CANVAS_WIDTH 320
#define CANVAS_HEIGHT 240

uint16_t color_map[64];
uint16_t bg_color;
uint16_t frame_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];

static inline uint16_t rgb888to565(unsigned char r, unsigned char g, unsigned char b) {
    uint16_t rgb565 = b >> 3;
    rgb565 |= (g >> 2) << 5;
    rgb565 |= (r >> 3) << 11;
    return rgb565;
} 

/* Wait until next allegro timer event is fired. */
void wait_for_frame()
{
    timer_fired = 0;
    while(!timer_fired);
}

/* Set background color. RGB value of c is defined in fce.h */
void nes_set_bg_color(int c)
{
    bg_color = color_map[c];
}

static inline void draw_pixel(int x, int y, uint16_t color)
{
    frame_buffer[y * CANVAS_WIDTH + x] = color;
}

/* Flush the pixel buffer */
void nes_flush_buf(PixelBuf *buf) {
    int i;
    int *fbuf = ((int *) frame_buffer);
    int bgc = bg_color | (bg_color << 16);
    for (i = 0; i < CANVAS_HEIGHT * CANVAS_WIDTH / 2; ++i) {
        fbuf[i] = bgc;
    }
    for (i = 0; i < buf->size; i ++) {
        Pixel *p = &buf->buf[i];
        int x = (p->xyc & 0xFFF00000) >> 20;
        int y = (p->xyc & 0xFFF00) >> 8;
        int cc = p->xyc & 0xFF;
        uint16_t c = color_map[cc];
        draw_pixel(x, y, c);
    }
}

void on_timer() {
	timer_fired = 1;
}

void trap_handler(void *epc, unsigned int cause) {
	if (cause == 0x80000007) {
		on_timer();
	} else {
		unsigned int ch = *UART_RECV;
		*UART_SEND = ch;
	}
}

void enable_interrupt();

/* Initialization:
   (1) start a 1/FPS Hz timer. 
   (2) register fce_timer handle on each timer event */
void nes_hal_init()
{
    for (int i = 0; i < 64; i ++) {
        pal color = palette[i];
        color_map[i] = rgb888to565(color.r, color.g, color.b);
    }
    enable_interrupt();
    *TIMER_LIMIT = REFRESH_TIMER_LIMIT;
    *TIMER_ENABLED = 1;
}

/* Update screen at FPS rate by allegro's drawing function. 
   Timer ensures this function is called FPS times a second. */
void nes_flip_display()
{
    nes_set_bg_color(bg_color);
    int *fbuf = ((int *) frame_buffer);
    int *vram = ((int *) VRAM);
    for (int i = 0; i < CANVAS_HEIGHT * CANVAS_WIDTH / 2; ++i) {
        vram[i] = fbuf[i];
    }
}

/* Query a button's state.
   Returns 1 if button #b is pressed. */
int nes_key_state(int b)
{
    return 0;
}

