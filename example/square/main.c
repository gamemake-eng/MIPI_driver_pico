#include <stdint.h>
#include <stdlib.h>
#include "../../src/include/mipi_driver.h"
#include "../../src/include/settings.h"
#include "../../src/include/mipi_dcs.h"
#include <pico/time.h>
#include <math.h>

#define DRAW_RECT(x,y,w,h) for(int i = 0; i < w; i++){for(int j = 0; j < h; j++){bb[((i+x)+(j+y)*DISPLAY_WIDTH)*MIPI_PIXEL_DEPTH/8] = 0;bb[((i+x)+(j+y)*DISPLAY_WIDTH)*MIPI_PIXEL_DEPTH/8+1] = 0;}}

int main(){
    uint8_t bb[(DISPLAY_WIDTH*DISPLAY_HEIGHT)*MIPI_PIXEL_DEPTH/8];

    init_mipi_display();

    float x=0;
    float y=0;

    int dx = 1;
	int dy = 1;

	float speed = 1.5;


    while(1){
        x += dx*speed;
		y += dy*speed;

		if(x <= 0){
			dx = 1;
		}else if (x>=160-16) {
			dx = -1;
		}

		if(y <= 0) {
			dy = 1;
		}else if (y >= 128-16){
			dy = -1;
		}

        for(int i = 0; i < (DISPLAY_WIDTH*DISPLAY_HEIGHT)*MIPI_PIXEL_DEPTH/8; i++){
            //Make display white
            bb[i] = 0xff;

        }

        //Draw a rectangle
        DRAW_RECT((uint8_t)floor(x), (uint8_t)floor(y), 16, 16);

        //Flip screen
        display_section_fill(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, bb);
    }

    return 0;
}
