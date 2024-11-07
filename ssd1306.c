/*

MIT License

Copyright (c) 2021 David Schramm

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ssd1306.h"
#include "font.h"

inline static void swap(int32_t *a, int32_t *b) {
    int32_t *t=a;
    *a=*b;
    *b=*t;
}

inline static void fancy_write(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, char *name) {
    switch(i2c_write_blocking(i2c, addr, src, len, false)) {
    case PICO_ERROR_GENERIC:
        printf("[%s] addr not acknowledged!\n", name);
        break;
    case PICO_ERROR_TIMEOUT:
        printf("[%s] timeout!\n", name);
        break;
    default:
        //printf("[%s] wrote successfully %lu bytes!\n", name, len);
        break;
    }
}

inline static void dandan_escribe(dandan_t *p, uint8_t val) {
    uint8_t d[2]= {0x00, val};
    fancy_write(p->i2c_i, p->address, d, 2, "dandan_escribe");
}

bool dandan_init(dandan_t *p, uint16_t ancho, uint16_t alto, uint8_t address, i2c_inst_t *i2c_instance) {
    p->ancho=ancho;
    p->alto=alto;
    p->pages=alto/8;
    p->address=address;

    p->i2c_i=i2c_instance;


    p->bufsize=(p->pages)*(p->ancho);
    if((p->buffer=malloc(p->bufsize+1))==NULL) {
        p->bufsize=0;
        return false;
    }

    ++(p->buffer);

    // from https://github.com/makerportal/rpi-pico-ssd1306
    uint8_t cmds[]= {
        SET_DISP,
        // timing and driving scheme
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        alto - 1,
        SET_DISP_OFFSET,
        0x00,
        // resolution and layout
        SET_DISP_START_LINE,
        // charge pump
        SET_CHARGE_PUMP,
        p->external_vcc?0x10:0x14,
        SET_SEG_REMAP | 0x01,           // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08,         // scan from COM[N] to COM0
        SET_COM_PIN_CFG,
        ancho>2*alto?0x02:0x12,
        // display
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        p->external_vcc?0x22:0xF1,
        SET_VCOM_DESEL,
        0x30,                           // or 0x40?
        SET_ENTIRE_ON,                  // output follows RAM contents
        SET_NORM_INV,                   // not inverted
        SET_DISP | 0x01,
        // address setting
        SET_MEM_ADDR,
        0x00,  // horizontal
    };

    for(size_t i=0; i<sizeof(cmds); ++i)
        dandan_escribe(p, cmds[i]);

    return true;
}

inline void dandan_deinit(dandan_t *p) {
    free(p->buffer-1);
}

inline void dandan_poweroff(dandan_t *p) {
    dandan_escribe(p, SET_DISP|0x00);
}

inline void dandan_poweron(dandan_t *p) {
    dandan_escribe(p, SET_DISP|0x01);
}

inline void dandan_contrast(dandan_t *p, uint8_t val) {
    dandan_escribe(p, SET_CONTRAST);
    dandan_escribe(p, val);
}

inline void dandan_invert(dandan_t *p, uint8_t inv) {
    dandan_escribe(p, SET_NORM_INV | (inv & 1));
}

inline void dandan_limpia (dandan_t *p) {
    memset(p->buffer, 0, p->bufsize);
}

void dandan_limpia_pixel (dandan_t *p, uint32_t x, uint32_t y) {
    if(x>=p->ancho || y>=p->alto) return;

    p->buffer[x+p->ancho*(y>>3)]&=~(0x1<<(y&0x07));
}

void dandan_escribe_pixel(dandan_t *p, uint32_t x, uint32_t y) {
    if(x>=p->ancho || y>=p->alto) return;

    p->buffer[x+p->ancho*(y>>3)]|=0x1<<(y&0x07); // y>>3==y/8 && y&0x7==y%8
}

void dandan_escribe_string(dandan_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s) {
    dandan_escribe_string_with_font(p, x, y, scale, font_8x5, s);
                                                    // font = arreglo de mapeo
}

void dandan_escribe(dandan_t *p) {
    uint8_t payload[]= {SET_COL_ADDR, 0, p->ancho-1, SET_PAGE_ADDR, 0, p->pages-1};
    if(p->ancho==64) {
        payload[1]+=32;
        payload[2]+=32;
    }

    for(size_t i=0; i<sizeof(payload); ++i)
        dandan_escribe(p, payload[i]);

    *(p->buffer-1)=0x40;

    fancy_write(p->i2c_i, p->address, p->buffer-1, p->bufsize+1, "dandan_mostrar");
}
