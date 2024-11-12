#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "dandan.h"
#include "papyrus_data.h"

const uint8_t num_chars_per_disp[]={7,7,7,5};
const uint8_t *fonts[]= {papyrus};


#define SLEEPTIME 25

void arranque(void);
void impre(void);

int main() {
    stdio_init_all();

    // arranque de pins de uso 
    arranque();

    // Impresion en pantalla oled 
    impre();

    return 0;
}


void arranque(void) {
    i2c_init(i2c1, 400000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);
}


void impre(void) {
    const char *words[]= {"Jose Isaac", "Suarez", "Guzman"};

    dandan_t disp;
    disp.external_vcc=false;
    dandan_init(&disp, 128,65, 0x3C, i2c1);

    dandan_limp(&disp);
    char buf[8];

    for(;;) {
        dandan_escribe_string(&disp,25,30,1,"JISG");
        dandan_mostrar(&disp);
        sleep_ms(1000);
        dandan_limp(&disp);
    }
}
