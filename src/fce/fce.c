#include "fce.h"
#include "cpu.h"
#include "memory.h"
#include "ppu.h"
#include "hal.h"
#include "nes.h"

PixelBuf bg, bbg, fg;

typedef struct {
    char signature[4];
    byte prg_block_count;
    byte chr_block_count;
    word rom_type;
    byte reserved[8];
} ines_header;

ines_header fce_rom_header;

// FCE Lifecycle

int fce_load_rom(char *prom)
{
    byte* rom = (byte*) prom;
    memcpy(&fce_rom_header, rom, sizeof(fce_rom_header));
    rom += sizeof(fce_rom_header);

    if (memcmp(fce_rom_header.signature, "NES\x1A", 4)) {
        return -1;
    }

    mmc_id = ((fce_rom_header.rom_type & 0xF0) >> 4);

    int prg_size = fce_rom_header.prg_block_count * 0x4000;

    if (mmc_id == 0 || mmc_id == 3) {
        // if there is only one PRG block, we must repeat it twice
        if (fce_rom_header.prg_block_count == 1) {
            mmc_copy(0x8000, rom, 0x4000);
            mmc_copy(0xC000, rom, 0x4000);
        }
        else {
            mmc_copy(0x8000, rom, 0x8000);
        }
    }
    else {
        return -1;
    }
    rom += prg_size;
    // Copying CHR pages into MMC and PPU
    int i;
    for (i = 0; i < fce_rom_header.chr_block_count; i++) {
        mmc_append_chr_rom_page(rom);

        if (i == 0) {
            ppu_copy(0x0000, rom, 0x2000);
        }
	    rom += 0x2000;
    }

    return 0;
}

void fce_init()
{
    nes_hal_init();
    cpu_init();
    ppu_init();
    ppu_set_mirroring(fce_rom_header.rom_type & 1);
    cpu_reset();
}

void wait_for_frame();

void fce_run()
{
    while(1)
    {
        wait_for_frame();
            int scanlines = 262;
            while (scanlines-- > 0)
            {
                ppu_run(1);
                cpu_run(1364 / 12); // 1 scanline
            }
    }
}

// Rendering


void fce_update_screen()
{
    int idx = ppu_ram_read(0x3F00);
    nes_set_bg_color(idx);
    
    if (ppu_shows_sprites())
        nes_flush_buf(&bbg);

    if (ppu_shows_background())
        nes_flush_buf(&bg);

    if (ppu_shows_sprites())
        nes_flush_buf(&fg);

    nes_flip_display();

    pixbuf_clean(bbg);
    pixbuf_clean(bg);
    pixbuf_clean(fg);
}

