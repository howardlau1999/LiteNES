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
#ifdef YATCPU
#include "mmio.h"
#endif 

#define REFRESH_TIMER_LIMIT 2083333
volatile int timer_fired = 0;

#define CANVAS_WIDTH 320
#define X_OFFSET 32
#define CANVAS_HEIGHT 240

uint32_t digits[16] = {
  0x69999996,
  0x22222222,
  0xE1168887,
  0xE116111E,
  0x99961111,
  0x7886111E,
  0x68869996,
  0xE1111111,
  0x69969996,
  0x69961116,
  0x69996999,
  0xE99E999E,
  0x78888887,
  0xE999999E,
  0x78878887,
  0x78868888,
};

uint32_t color_map[64];
uint32_t bg_color;
uint32_t frame_buffer[CANVAS_WIDTH * CANVAS_HEIGHT];

static inline uint16_t rgb888to565(unsigned char r, unsigned char g, unsigned char b) {
    uint16_t rgb565 = b >> 3;
    rgb565 |= (g >> 2) << 5;
    rgb565 |= (r >> 3) << 11;
    return rgb565;
} 

static inline uint32_t packrgb(unsigned char r, unsigned char g, unsigned char b) {
    return (r << 16) | (g << 8) | b;
}

#ifdef LITENES_DEBUG
#define EMU_FRAMES 600
#endif

int frames = 0;

/* Wait until next allegro timer event is fired. */
void wait_for_frame()
{
    #ifdef YATCPU
    // timer_fired = 0;
    // while(!timer_fired);
    #endif
}

/* Set background color. RGB value of c is defined in fce.h */
void nes_set_bg_color(int c)
{
    bg_color = color_map[c];
}

static void draw_digit(int x, int y, int digit) {
    uint32_t bits = digits[digit];
    for (int h = 0; h < 8; ++h) {
        for (int w = 0; w < 4; ++w) {
            if (bits & (1 << ((7 - h) * 4 + (3 - w)))) {
                frame_buffer[(y + h) * CANVAS_WIDTH + (x + w)] = 0xFFFFFFFF;
            }
        }
    }
}

static void draw_frame_counter(int x, int y) {
    int frame_count = frames;
    for (int i = 0; i < 8; ++i) {
        int digit = ((0xF << (i * 4)) & frame_count) >> (i * 4);
        draw_digit(x + (7 - i) * 5, y, digit);
    }
}

/* Flush the pixel buffer */
void nes_flush_buf(PixelBuf *buf) {
    int i;
    
    for (i = 0; i < buf->size; i ++) {
        Pixel *p = &buf->buf[i];
        int x = ((p->xyc & 0xFFF00000) >> 20) + X_OFFSET;
        int y = (p->xyc & 0xFFF00) >> 8;
        int cc = p->xyc & 0xFF;
        uint32_t c = color_map[cc];
        frame_buffer[y * CANVAS_WIDTH + x] = c;
    }
    buf->size = 0;
}

#ifdef YATCPU
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
#endif 

/* Initialization:
   (1) start a 1/FPS Hz timer. 
   (2) register fce_timer handle on each timer event */
void nes_hal_init()
{
    for (int i = 0; i < 64; i ++) {
        pal color = palette[i];
        color_map[i] = packrgb(color.r, color.g, color.b);
    }
    #ifdef YATCPU
    int *vram = ((int *) VRAM);
    for (int y = 0; y < CANVAS_HEIGHT; ++y) {
        for (int x = 0; x < 32; ++x) {
            vram[y * CANVAS_WIDTH + x] = 0;
        }
        for (int x = 288; x < 320; ++x) {
            vram[y * CANVAS_WIDTH + x] = 0;
        }
    }
    draw_frame_counter(32, 4);
    // enable_interrupt();
    // *TIMER_LIMIT = REFRESH_TIMER_LIMIT;
    // *TIMER_ENABLED = 1;
    #endif
}

/* Update screen at FPS rate by allegro's drawing function. 
   Timer ensures this function is called FPS times a second. */
void nes_flip_display()
{
    int bgc = bg_color;
    draw_frame_counter(32, 4);
    #ifdef YATCPU
    int *fbuf = ((int *) frame_buffer);
    int *vram = ((int *) VRAM);
    for (int y = 0; y < CANVAS_HEIGHT; ++y) {
        for (int x = 32; x < 288; ++x) {
            int i = y * CANVAS_WIDTH + x;
            vram[i] = fbuf[i];
            fbuf[i] = bgc;
        }
    }
    ++frames;
    #endif
    #ifdef LITENES_DEBUG
    char filename[32];
    snprintf(filename, 32, "frame_%d.rgb888", frames);
    FILE* fp = fopen(filename, "wb");
    fwrite(frame_buffer, CANVAS_WIDTH * CANVAS_HEIGHT * 4, 1, fp);
    fclose(fp);
    int* fbuf = ((int*)frame_buffer);
    for (int i = 0; i < CANVAS_HEIGHT * CANVAS_WIDTH; ++i) {
        fbuf[i] = bgc;
    }

    if (frames >= EMU_FRAMES) {
        exit(0);
    }
    ++frames;
    printf("Emulating frame %d\n", frames);
    #endif
}

/* Query a button's state.
   Returns 1 if button #b is pressed. */
int nes_key_state(int b)
{
    #ifdef YATCPU
    #endif
    return 0;
}

