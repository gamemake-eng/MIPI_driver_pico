#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "../../src/include/mipi_driver.h"
#include "../../src/include/settings.h"
#include "../../src/include/mipi_dcs.h"
#include <pico/time.h>
#include <math.h>

#define DRAW_RECT(x,y,w,h,bb) for(int i = 0; i < w; i++){for(int j = 0; j < h; j++){bb[((i+x)+(j+y)*DISPLAY_WIDTH)*MIPI_PIXEL_DEPTH/8] = 0;bb[((i+x)+(j+y)*DISPLAY_WIDTH)*MIPI_PIXEL_DEPTH/8+1] = 0;}}

int main(){
	setup_default_uart();
	
 	uint8_t bb[(DISPLAY_WIDTH*DISPLAY_HEIGHT)*MIPI_PIXEL_DEPTH/8];
	uint8_t bb2[(DISPLAY_WIDTH*DISPLAY_HEIGHT)*MIPI_PIXEL_DEPTH/8];
	uint8_t buffer = 0;

	init_mipi_display();

	float x=0;
	float y=0;

	int dx = 1;
	int dy = 1;

	float speed = 1.5;


	while(1){
		//gpio_put(25,1);
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
			if(buffer == 0){
	    			bb[i] = 0xFF;
			}
			else{
				bb2[i] = 0xFF;
			}

		}

		//Draw a rectangle
		//DRAW_RECT((uint8_t)floor(x), (uint8_t)floor(y), 16, 16,bb);

		if(buffer==0){
			memcpy(bb2, bb, sizeof(bb));
			buffer=1;
		}else {
			memcpy(bb, bb2, sizeof(bb2));   
			buffer=0;
		}
		//Flip screen
		if(buffer == 0){
			//Draw a rectangle
                	DRAW_RECT((uint8_t)floor(x), (uint8_t)floor(y), 16, 16,bb);   
			display_section_fill(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, bb);
		}
		else{
			//Draw a rectangle
                	DRAW_RECT((uint8_t)floor(x), (uint8_t)floor(y), 16, 16,bb2);   
			display_section_fill(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, bb2);  
		}

	}

	return 0;
}
