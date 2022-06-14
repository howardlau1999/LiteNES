/*
LiteNES originates from Stanislav Yaglo's mynes project:
  https://github.com/yaglo/mynes

LiteNES is a "more portable" version of mynes.
  all system(library)-dependent code resides in "hal.c" and "main.c"
  only depends on libc's memory moving utilities.

How does the emulator work?
  1) read file name at argv[1]
  2) load the rom file into array rom
  3) call fce_load_rom(rom) for parsing
  4) call fce_init for emulator initialization
  5) call fce_run(), which is a non-exiting loop simulating the NES system
  6) when SIGINT signal is received, it kills itself
*/

#include "fce.h"
#ifdef YATCPU
#include "mmio.h"
#endif

extern char rom[];

extern const int left_border_end;
extern const int right_border_start;
extern const int right_border_end;

unsigned int rgb888to565(unsigned char r, unsigned char g, unsigned char b); 
unsigned short packrgb(unsigned char r, unsigned char g, unsigned char b);

#ifdef RGB888
unsigned int *vram = (unsigned int *) VRAM;
#else
unsigned short *vram = (unsigned short *) VRAM;
#endif

void delay(int count) {
  while(count--);
}

int main(int argc, char *argv[])
{
    #ifdef YATCPU 
    int *bss_start = (int*) 0x00100000;
    int *bss_end = (int*) 0x01000000;
    for (int *p = bss_start; p < bss_end; ++p) {
        *p = 0;
    }

    for (int y = 0; y < 240; ++y) {
      for (int x = 0; x < 320; ++x) {
        int i = y * 320 + x;
        int r = (x & 0xF) << 4;
        int g = (x & 0xF0);
        int b = 0xFF;
        #ifdef RGB888
        vram[i] = rgbpack(r, g, b);
        #else
        vram[i] = rgb888to565(r, g, b);
        #endif
      }
    }

    delay(100000);

    #endif
    int res = fce_load_rom(rom);
    #ifdef YATCPU
    unsigned int c;
    #ifdef RGB888
    c = rgbpack(0, 255, 0);
    #else
    c = rgb888to565(0, 255, 0);
    #endif
    for (int i = 0; i < 320 * 240; ++i) {
      vram[i] = c;
    }
    #endif
    #ifdef LITENES_DEBUG
    if (res != 0) {
      printf("Error: failed to load rom file.\n");
      return -1;
    }
    #endif
    #ifdef LITENES_DEBUG
      printf("ROM Loaded.\n");
    #endif
    fce_init();
    #ifdef LITENES_DEBUG
      printf("FCE initialized.\n");
    #endif
    fce_run();
    return 0;
}
