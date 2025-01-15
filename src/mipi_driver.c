//I am mostly using tuupola's hagl mipi backend repo (https://github.com/tuupola/hagl_pico_mipi) as a reference

#include <stdlib.h>
#include <stdio.h>

#ifdef MIPI_USE_DMA
#include <hardware/dma.h>
#endif

#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/time.h>

#include "include/mipi_driver.h"
#include "include/mipi_dcs.h"
#include "include/settings.h"

inline uint16_t htons(uint16_t i){
	//reverse word in a single instruction
	__asm ("rev16 %0, %0" : "+l" (i) : : );
	return i;
}

#ifdef MIPI_USE_DMA
static volatile uint dma_c;
static dma_channel_config dma_c_config;
#endif

void send_mipi_command(uint8_t cmd){
	//As seen in page 30 of the st7735 datasheet, you set DC to low for commands
	gpio_put(DISPLAY_DC, 0);

	//Set chip select to low to enable spi device
	gpio_put(DISPLAY_CS, 0);

	spi_write_blocking(DISPLAY_SPI_PORT, &cmd, 1);

	//Disable spi device by setting CS to high
	gpio_put(DISPLAY_CS, 1);
}

void send_mipi_data(uint8_t *data, size_t len){

	//Return if length is equal to zero since it wouldn't send any data anyway
	if(len == 0){
		return;
	}

	//As seen in page 30 of the st7735 datasheet, you set DC to high for data
	gpio_put(DISPLAY_DC, 1);

	//Set CS to low to enable spi device
	gpio_put(DISPLAY_CS, 0);

	//#ifdef MIPI_USE_DMA
	//spi_write_blocking(DISPLAY_SPI_PORT, data, len);
	//#else

	//Loop though list and send data though the data register
	for (size_t i=0; i<len; i++){
			//wait until you can write into the spi port
			while(!spi_is_writable(DISPLAY_SPI_PORT)){}
			spi_get_hw(DISPLAY_SPI_PORT)->dr = (uint32_t) data[i];
	}


	//Wait for shifting to finish
	while (spi_get_hw(DISPLAY_SPI_PORT)->sr & SPI_SSPSR_BSY_BITS) {}
	spi_get_hw(DISPLAY_SPI_PORT)->icr = SPI_SSPICR_RORIC_BITS;
	//I should probably research what this does
	//#endif

	//Disable spi device by setting CS to high
	gpio_put(DISPLAY_CS, 1);
}

void set_address_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	uint8_t data[4];

	//Make static to be able to keep their values when out of scope
	static uint16_t prev_x1, prev_x2, prev_y1, prev_y2;

	//Offset values so you can fill the entire screen
	x1 = x1 + DISPLAY_OFFSET_X;
	y1 = y1 + DISPLAY_OFFSET_Y;
	x2 = x2 + DISPLAY_OFFSET_X;
	y2 = y2 + DISPLAY_OFFSET_Y;

	//Only change addresses when it has changed

	if((prev_x1 != x1) || (prev_x2 != x2)){
			send_mipi_command(MIPI_DCS_SET_COLUMN_ADDRESS);
			//Seperate words into 2 bytes
			data[0] = x1 >> 8;
			data[1] = x1 & 0xff;
			data[2] = x2 >> 8;
			data[3] = x2 & 0xff;

			send_mipi_data(data, 4);

			prev_x1 = x1;
			prev_x2 = x2;
	}

	if((prev_y1 != y1) || (prev_y2 != y2)){
			send_mipi_command(MIPI_DCS_SET_PAGE_ADDRESS);
			//Seperate words into 2 bytes
			data[0] = y1 >> 8;
			data[1] = y1 & 0xff;
			data[2] = y2 >> 8;
			data[3] = y2 & 0xff;

			send_mipi_data(data, 4);

			prev_y1 = y1;
			prev_y2 = y2;
	}

	send_mipi_command(MIPI_DCS_WRITE_MEMORY_START);


}

#ifdef MIPI_USE_DMA
void init_mipi_dma() {
	dma_c = dma_claim_unused_channel(true);
	dma_c_config = dma_channel_get_default_config(dma_c);
	channel_config_set_transfer_data_size(&dma_c_config, DMA_SIZE_8);
	channel_config_set_read_increment(&dma_c_config, true);
	channel_config_set_write_increment(&dma_c_config, false);
	channel_config_set_ring(&dma_c_config, false, 0);	
	channel_config_set_dreq(&dma_c_config, spi_get_index(DISPLAY_SPI_PORT) ? DREQ_SPI1_TX : DREQ_SPI0_TX);
	
		

}
void send_spi_dma(uint8_t *buffer, size_t len){
	//data mode
	gpio_put(DISPLAY_DC, 1);

	//Enable spi device
	gpio_put(DISPLAY_CS, 0);

	dma_channel_configure(dma_c, &dma_c_config, &spi_get_hw(DISPLAY_SPI_PORT)->dr, buffer, len, true);
	dma_channel_wait_for_finish_blocking(dma_c);
	
	#ifdef DEBUG_MIPI
	gpio_put(25, 1);
	#endif
}
#endif

void display_section_fill(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint8_t *buffer) {
	int32_t x2 = x1 + w - 1;
	int32_t y2 = y1 + h - 1;

	set_address_window(x1, y1, x2, y2);

	#ifdef DEBUG_MIPI
	//Turn off built in led before frame is drawn
	gpio_put(25, 0);
	#endif
	
	//Send data to display
	#ifdef MIPI_USE_DMA
	send_spi_dma(buffer, (w*h)*MIPI_PIXEL_DEPTH/8);
	#else
	send_mipi_data(buffer, (w*h)*MIPI_PIXEL_DEPTH/8);
	#endif
}


void init_mipi_spi() {
	//Enable DC and CS pins
	gpio_set_function(DISPLAY_DC, GPIO_FUNC_SIO);
	gpio_set_dir(DISPLAY_DC, GPIO_OUT);

	gpio_set_function(DISPLAY_CS, GPIO_FUNC_SIO);
	gpio_set_dir(DISPLAY_CS, GPIO_OUT);

	//Enable spi pins
	gpio_set_function(DISPLAY_SCK, GPIO_FUNC_SPI);
	gpio_set_function(DISPLAY_SDA, GPIO_FUNC_SPI);

	//Set CS to high to disable spi device
	gpio_put(DISPLAY_CS, 1);

	//Init spi device
	spi_init(DISPLAY_SPI_PORT, SPI_CLOCK_SPEED);
	spi_set_format(DISPLAY_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

}

void init_mipi_display() {

	init_mipi_spi();
	sleep_ms(100);

	#ifdef DEBUG_MIPI
	gpio_set_function(25, GPIO_FUNC_SIO);
	gpio_set_dir(25, GPIO_OUT);
	#endif

	//Hardware reset display
	if(DISPLAY_RST > 0){
		gpio_set_function(DISPLAY_RST, GPIO_FUNC_SIO);
		gpio_set_dir(DISPLAY_RST, GPIO_OUT);

		gpio_put(DISPLAY_RST, 0);
		sleep_ms(100);
		gpio_put(DISPLAY_RST, 1);
		sleep_ms(100);
	}

	//Send init commands
	send_mipi_command(MIPI_DCS_SOFT_RESET);
	sleep_ms(200);

	//set address mode
	send_mipi_command(MIPI_DCS_SET_ADDRESS_MODE);
	send_mipi_data(&(uint8_t) {MIPI_ADDRESS_MODE}, 1);

	//set color mode
	send_mipi_command(MIPI_DCS_SET_PIXEL_FORMAT);
	send_mipi_data(&(uint8_t) {MIPI_PIXEL_FORMAT}, 1);

	//set display inversion
	#if DISPLAY_INVERT
	send_mipi_command(MIPI_DCS_ENTER_INVERT_MODE);
	#else
	send_mipi_command(MIPI_DCS_EXIT_INVERT_MODE);
	#endif

	//Turn display on
	send_mipi_command(MIPI_DCS_EXIT_SLEEP_MODE);
	sleep_ms(200);
	send_mipi_command(MIPI_DCS_SET_DISPLAY_ON);
	sleep_ms(200);

	//Set address window to entire display
	set_address_window(0, 0, DISPLAY_WIDTH-1, DISPLAY_HEIGHT-1);

	#ifdef MIPI_USE_DMA
	init_mipi_dma();
	#endif

}

void close_mipi_display() {
	spi_deinit(DISPLAY_SPI_PORT);
}
