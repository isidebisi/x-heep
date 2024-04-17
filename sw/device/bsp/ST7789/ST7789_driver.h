#ifndef ST7789_DRIVER_H
#define ST7789_DRIVER_H

/*
 * Include files
 */
#include <stdint.h>
#include "spi_host_regs.h"
#include "spi_host.h"
#include "gpio.h"

#include "core_v_mini_mcu.h"


/*
 * Global variables
 */


/* 
 * Function prototypes
 */
uint8_t spi_init(void);
uint8_t display_init(void);
static void configure_spi(void);
void spi_write_command(uint8_t command);
void spi_write_data(uint8_t data);
void spi_write_data_2B(uint16_t data);

void set_adress_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
uint32_t test_write_pixel(uint16_t x, uint16_t y, uint16_t color);
void test_write_multi_unicolor(uint16_t color, uint32_t num);
void test_fill_screen(uint16_t color);
void fill_picture(uint16_t* colors);

void milli_delay(int n_milli_seconds);

/*
 * Defines
 */

// ST7789 commands

#define ST7789_TFTWIDTH 	240
#define ST7789_TFTHEIGHT 	240

#define ST7789_NOP     0x00
#define ST7789_SWRESET 0x01

#define ST7789_SLPIN   0x10  // sleep on
#define ST7789_SLPOUT  0x11  // sleep off
#define ST7789_PTLON   0x12  // partial on
#define ST7789_NORON   0x13  // partial off
#define ST7789_INVOFF  0x20  // invert off
#define ST7789_INVON   0x21  // invert on
#define ST7789_DISPOFF 0x28  // display off
#define ST7789_DISPON  0x29  // display on
#define ST7789_IDMOFF  0x38  // idle off
#define ST7789_IDMON   0x39  // idle on

#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_RAMRD   0x2E

#define ST7789_COLMOD  0x3A
#define ST7789_COLOR_MODE_65K 0x50 // 65k color
#define ST7789_COLOR_MODE_16BIT 0x05 // 16-bit color
#define ST7789_MADCTL  0x36

#define ST7789_PTLAR    0x30   // partial start/end
#define ST7789_VSCRDEF  0x33   // SETSCROLLAREA
#define ST7789_VSCRSADD 0x37

#define ST7789_WRDISBV  0x51
#define ST7789_WRCTRLD  0x53
#define ST7789_WRCACE   0x55
#define ST7789_WRCABCMB 0x5e

#define ST7789_POWSAVE    0xbc
#define ST7789_DLPOFFSAVE 0xbd

// bits in MADCTL
#define ST7789_MADCTL_MY  0x80
#define ST7789_MADCTL_MX  0x40
#define ST7789_MADCTL_MV  0x20
#define ST7789_MADCTL_ML  0x10
#define ST7789_MADCTL_RGB 0x00

#define ST7789_240x240_XSTART 0
#define ST7789_240x240_YSTART 0

#define ST_CMD_DELAY   0x80

#endif // ST7789_DRIVER_H
