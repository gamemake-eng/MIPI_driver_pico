//I am mostly using tuupola's hagl mipi backend repo (https://github.com/tuupola/hagl_pico_mipi) as a reference

#include <cstddef>
#include <cstdint>
#include <stdlib.h>

#include <hardware/spi.h>
#include <hardware/gpio.h>
#include <pico/time.h>

#include "mipi_driver.h"
#include "include/mipi_driver.h"
#include "mipi_dcs.h"
#include "settings.h"

inline uint16_t htons(uint16_t i){
    //reverse word in a single instruction
    __asm ("rev16 %0, %0" : "+l" (i) : : );
    return i;
}

void send_mipi_command(uint8_t cmd){
    //As seen in page 30 of the st7735 datasheet, you set DC to low for commands
    gpio_put(DISPLAY_DC, 0);

    //Set chip select to low to enable spi device
    gpio_put(DISPLAY_CS, 0);

    spi_write_blocking(DISPLAY_SPI_PORT, &command, 1);

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

    //Loop though list and send data though the data register
    for (size_t i=0; i<len; i++){
        //wait until you can write into the spi port
        while(!spi_is_writable(DISPLAY_SPI_PORT)){}
        spi_get_hw(DISPLAY_SPI_PORT)->dr = (uint32_t) data[i];
    }

    //Wait for shifting to finish
    while (spi_get_hw(MIPI_DISPLAY_SPI_PORT)->sr & SPI_SSPSR_BSY_BITS) {}
    spi_get_hw(DISPLAY_SPI_PORT)->icr = SPI_SSPICR_RORIC_BITS;
    //I should probably research what this does

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

void push_pixels(void *_color, size_t len) {
    size_t size = len;
    uint16_t *color = _color;

    //Set DC to high to enable data transfer
    gpio_put(DISPLAY_DC, 1);
    //Set CS to low to enable spi device
    gpio_put(DISPLAY_CS, 0);
    //Set spi format to whatever is set (default is 16-bit)
    spi_set_format(DISPLAY_SPI_PORT, MIPI_PIXEL_DEPTH, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    //Loop through the list and send data through the data register
    while(size--){
        //wait until you can write into the spi port
        while(!spi_is_writable(DISPLAY_SPI_PORT)){}
        spi_get_hw(DISPLAY_SPI_PORT)->dr = (uint32_t) htons(*color);
    }

    //Wait for shifting to finish
    while (spi_get_hw(DISPLAY_SPI_PORT)->sr & SPI_SSPSR_BSY_BITS) {}
    spi_get_hw(DISPLAY_SPI_PORT)->icr = SPI_SSPICR_RORIC_BITS;

    //Set the spi format back to 8-bit
    spi_set_format(DISPLAY_SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    //Disable spi device by setting CS to high
    gpio_put(DISPLAY_CS, 1);

}

void display_section_fill(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, void *color) {
    int32_t x2 = x1 + w - 1;
    int32_t y2 = y1 + h - 1;

    set_address_window(x1, y1, x2, y2);

    push_pixels(*color, w*h);
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
    send_mipi_command(MIPI_DCS_SET_PIXEL_MODE);
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

}

void close_mipi_display() {
    spi_deinit(DISPLAY_SPI_PORT);
}
