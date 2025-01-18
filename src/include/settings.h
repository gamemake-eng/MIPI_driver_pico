//Make sure header is only included once
#ifndef _SETTINGS_H
#define _SETTINGS_H

//C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

#include "mipi_dcs.h"

//A bunch of defines you can set in the Cmake file

#ifndef DISPLAY_SCK
#define DISPLAY_SCK 18
#endif

#ifndef DISPLAY_SDA
#define DISPLAY_SDA 19
#endif

//Also known as A0 on some boards
#ifndef DISPLAY_DC
#define DISPLAY_DC 20
#endif

#ifndef DISPLAY_RST
#define DISPLAY_RST 21
#endif

#ifndef DISPLAY_CS
#define DISPLAY_CS 17
#endif

#ifndef DISPLAY_SPI_PORT
#define DISPLAY_SPI_PORT spi0
#endif

#ifndef SPI_CLOCK_SPEED
#define SPI_CLOCK_SPEED 62500000
#endif

#ifndef DISPLAY_OFFSET_X
#define DISPLAY_OFFSET_X 1
#endif

#ifndef DISPLAY_OFFSET_Y
#define DISPLAY_OFFSET_Y 2
#endif

#ifndef MIPI_ADDRESS_MODE
//Make display horizontal mode by default
#define MIPI_ADDRESS_MODE MIPI_DCS_ADDRESS_MODE_RGB|MIPI_DCS_ADDRESS_MODE_SWAP_XY|MIPI_DCS_ADDRESS_MODE_MIRROR_X
#endif

#ifndef MIPI_PIXEL_FORMAT
//16-bit color
#define MIPI_PIXEL_FORMAT MIPI_DCS_PIXEL_FORMAT_16BIT
#endif

#ifndef MIPI_PIXEL_DEPTH
#define MIPI_PIXEL_DEPTH 16
#endif


#ifndef DISPLAY_WIDTH
#define DISPLAY_WIDTH 160
#endif

#ifndef DISPLAY_HEIGHT
#define DISPLAY_HEIGHT 128
#endif

#ifndef DISPLAY_INVERT
#define DISPLAY_INVERT 0
#endif


//C++ compatibility
#ifdef __cplusplus
}
#endif


#endif
