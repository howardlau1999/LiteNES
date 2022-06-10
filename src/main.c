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
#define CANVAS_WIDTH 320
#define X_OFFSET 32
#define CANVAS_HEIGHT 240
int main(int argc, char *argv[])
{
    #ifdef YATCPU 
    int *vram = ((int *) VRAM);
    for (int i = 0; i < 320 * 240; ++i) {
        vram[i] = 0xFFFFFFFF;
    }
    #endif
    int res = fce_load_rom(rom);
    #ifdef YATCPU
    for (int i = 0; i < 320 * 240; ++i) {
        vram[i] = 0x000000FF;
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
    #ifdef YATCPU
        for (int y = 0; y < CANVAS_HEIGHT; ++y) {
        for (int x = 32; x < 288; ++x) {
            vram[y * CANVAS_WIDTH + x] = 0x0000FF00;
        }
    }
    #endif
    #ifdef LITENES_DEBUG
      printf("FCE initialized.\n");
    #endif
    fce_run();
    return 0;
}
