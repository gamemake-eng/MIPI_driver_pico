#include <stdint.h>
#include <stdlib.h>
#include "../../src/include/mipi_driver.h"
#include "../../src/include/settings.h"
#include "../../src/include/mipi_dcs.h"
#include <pico/time.h>

int main(){
    uint8_t bb[(DISPLAY_WIDTH*DISPLAY_HEIGHT)*MIPI_PIXEL_DEPTH/8];
    init_mipi_display();

    uint16_t b = 0;

    for(int i = 0; i < (DISPLAY_WIDTH*DISPLAY_HEIGHT)*MIPI_PIXEL_DEPTH/8; i+=2){
        //Make display colorful
        bb[i] = b>>8;
        bb[i+1] = b&0xff;
        if(b == 0xffff){
            b = 0;
        }else{
            b++;
        }

    }

    size_t len = 0;

    while(1){
        display_section_fill(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, bb);
    }

    return 0;
}
