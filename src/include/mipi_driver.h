//Make sure header is only included once
#ifndef _MIPI_DRIVER_H
#define _MIPI_DRIVER_H

//C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>

void send_mipi_command(uint8_t cmd);
//Make len param a size_t type to make sure it's an array index
void send_mipi_data(uint8_t *data, size_t len);

void set_address_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void push_pixels(void *_color, size_t len);

void display_section_fill(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, void *color);

void init_mipi_display();
void close_mipi_display();


//C++ compatibility
#ifdef __cplusplus
}
#endif


#endif
