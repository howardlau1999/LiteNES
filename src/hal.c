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


void memcpy(char* dst, char* src, int size) 
{
    int rem = size & 0x3;
    int words = size >> 2;
    int* word_dst = (int*) dst;
    int* word_src = (int*) src;
    for (int i = 0; i < words; ++i) {
        *(word_dst++) = *(word_src++);
    }
    char *char_dst = (char*) word_dst;
    char *char_src = (char*) word_src;
    for (int i = 0; i < rem; ++i) {
        *(char_dst++) = *(char_src++);
    }
}

int memcmp(char* a, char* b, int size) {
    for (int i = 0; i < size; ++i) {
        int result = *a - *b;
        if (result != 0) return result;
        ++a;
        ++b;
    }
    return 0;    
}

/* Wait until next allegro timer event is fired. */
void wait_for_frame()
{
    while (1)
    {
    }
}

/* Set background color. RGB value of c is defined in fce.h */
void nes_set_bg_color(int c)
{
}

/* Flush the pixel buffer */
void nes_flush_buf(PixelBuf *buf)
{
}

/* Initialization:
   (1) start a 1/FPS Hz timer. 
   (2) register fce_timer handle on each timer event */
void nes_hal_init()
{
}

/* Update screen at FPS rate by allegro's drawing function. 
   Timer ensures this function is called FPS times a second. */
void nes_flip_display()
{
}

/* Query a button's state.
   Returns 1 if button #b is pressed. */
int nes_key_state(int b)
{
    return 1;
}

