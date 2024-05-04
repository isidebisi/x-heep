/* inspiration from:
 * Arduino_ST7789_Fast 
 * https://github.com/cbm80amiga/Arduino_ST7789_Fast/tree/b2782a381d61511b87df6cd6b20b71276d072a6d
 * 
 * stm32f1_st7789_spi
 * https://github.com/abhra0897/stm32f1_st7789_spi/tree/master
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "filtered_array.h"
/* To get TX and RX FIFO depth */
#include "spi_host_regs.h"

/* To get the target of the compilation (sim or pynq) */
#include "x-heep.h"

/* To get the soc_ctrl base address */
#include "soc_ctrl_structs.h"
#include "spi_host.h"

/* get the GPIO library*/
#include "gpio.h" 

#include "ST7789_driver.h"

#include "core_v_mini_mcu.h"




// AR0 = DC
// AR2 = RST (NOT USED)
// AR6 = CS
// AR4 = SCLK
// AR8 = MOSI




/* By default, PRINTFs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#define TARGET_PYNQ_Z2 1

#if TARGET_SIM && PRINTF_IN_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

/**
 * @brief SPI structure.
*/
spi_host_t ST7789_spi_LCD; 


void        ST7789_gpio_init(void);
uint8_t     ST7789_spi_init(void);
uint8_t     ST7789_display_init(void);
void        ST7789_spi_write_command(uint8_t command);
void        ST7789_spi_write_data(uint8_t data);
void        ST7789_spi_write_data_2B(uint16_t data);

void        ST7789_set_adress_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
uint32_t    ST7789_test_write_pixel(uint16_t x, uint16_t y, uint16_t color);
void        ST7789_test_write_multi_unicolor(uint16_t color, uint32_t num);
void        ST7789_test_fill_screen(uint16_t color);
void        ST7789_fill_picture(uint16_t* colors);
void        ST7789_test_fill_picture_with_shift(uint16_t* colors, uint8_t verticalShift, uint8_t horizontalShift);

void        ST7789_milli_delay(int n_milli_seconds);
static void ST7789_configure_spi(void);





int main(int argc, char *argv[]) {

    PRINTF("START PROGRAM\n");

    ST7789_gpio_init();

    PRINTF("SPI INIT\n");
    

    ST7789_spi_init();
    ST7789_display_init();

    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t color = 0xA000;

    ST7789_test_fill_screen(0x8000);

    int it =0;
    int a = 2;

    while(1)
    {
        ST7789_test_fill_picture_with_shift(&filtered_array,it,it);
        PRINTF("LOOP FINISHED\n");
        it += a;
        if (it>=16) a=-2;
        if (it<=0) a=2;

        //milli_delay(500);
    }

    while(0)
    {
        ST7789_fill_picture(&filtered_array);
        PRINTF("LOOP FINISHED\n");
        ST7789_milli_delay(500);
    }

    while(1)
    {
        ST7789_test_fill_screen(color);
        PRINTF("Fill with color: %d\n", color);
        color += 0x0003;
        PRINTF("LOOP FINISHED\n");
        ST7789_milli_delay(500);
    }

    while(1){
        PRINTF("RESTART with color: %d\n", color);
        color += 0x0000;
        x = 140;
        y = 30;

        while (x<240)
        {
            PRINTF("x: %d\n", x);

            while (y<240)
            {
                PRINTF("y: %d\n", y);
                ST7789_test_write_pixel(x, y, color);
                ST7789_milli_delay(1);
                y++;
            }
            y=30;
            x++;
        }
    }

    
    

}



void ST7789_gpio_init(void)
{
        gpio_cfg_t cfg_out = {
        .pin = GPIO_SPI_DC,
        .mode = GpioModeOutPushPull
    };
    gpio_config(cfg_out);
    
    gpio_cfg_t cfg_out2 = {
        .pin = GPIO_SPI_RST,
        .mode = GpioModeOutPushPull
    };
    gpio_config(cfg_out2);    

}

uint8_t ST7789_spi_init(void) {

    ST7789_spi_LCD.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    
        // Enable SPI host device
    spi_set_enable(&ST7789_spi_LCD, true);

    // Enable SPI output
    spi_output_enable(&ST7789_spi_LCD, true);

    // Configure SPI connection on CSID 0
    ST7789_configure_spi();

    // Set CSID
    spi_set_csid(&ST7789_spi_LCD, 0);
    ST7789_milli_delay(100);

    return 0;

}

uint8_t ST7789_display_init(void)
{
    //display reset
    ////PRINTF("Display Initialization \n");
    gpio_write(GPIO_SPI_RST, 1);
    ST7789_milli_delay(100);
    gpio_write(GPIO_SPI_RST, 0);
    ST7789_milli_delay(100);
    gpio_write(GPIO_SPI_RST, 1);
    ST7789_milli_delay(100);

    gpio_write(GPIO_SPI_DC, DC_COMMAND);

    ST7789_spi_write_command(ST7789_SWRESET);	//1: Software reset, no args, w/delay: delay(150)
    //PRINTF("ST7789_SWRESET 0x01\n");
    ST7789_milli_delay(150);

    ST7789_spi_write_command(ST7789_SLPOUT);	// 2: Out of sleep mode, no args, w/delay: delay(500)
    //PRINTF("ST7789_SLPOUT 0x11\n");
	ST7789_milli_delay(500);

	ST7789_spi_write_command(ST7789_COLMOD);	// 3: Set color mode, 1 arg, delay: delay(10)
    //PRINTF("ST7789_COLMOD 0x3A\n");

    ST7789_spi_write_data(ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT);	// 65K color, 16-bit color
    //PRINTF("ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT = 0x55\n");
	ST7789_milli_delay(150);

    ST7789_spi_write_command(ST7789_MADCTL);	// 4: Memory access ctrl (directions), 1 arg:
    //PRINTF("ST7789_MADCTL 0x36\n");

	ST7789_spi_write_data(ST7789_MADCTL_RGB);	// RGB Color
    //PRINTF("ST7789_MADCTL_RGB = 0x00\n");

    ST7789_spi_write_command(ST7789_CASET);	// 5: Column addr set, 4 args, no delay:
    //PRINTF("ST7789_CASET 0x2A\n");
    ST7789_spi_write_data(ST7789_240x240_XSTART >> 8);
    ST7789_spi_write_data(ST7789_240x240_XSTART);
    ST7789_spi_write_data((ST7789_TFTWIDTH + ST7789_240x240_XSTART) >> 8);
    ST7789_spi_write_data((ST7789_TFTWIDTH + ST7789_240x240_XSTART));

    ST7789_spi_write_command(ST7789_RASET);	// 6: Row addr set, 4 args, no delay:
    //PRINTF("ST7789_RASET 0x2B\n");
    ST7789_spi_write_data(ST7789_240x240_YSTART >> 8);
    ST7789_spi_write_data(ST7789_240x240_YSTART);
    ST7789_spi_write_data((ST7789_TFTHEIGHT + ST7789_240x240_YSTART) >> 8);
    ST7789_spi_write_data((ST7789_TFTHEIGHT + ST7789_240x240_YSTART));

    ST7789_spi_write_command(ST7789_INVON);	// 5: Inversion ON (but why?) delay(10)
    //PRINTF("ST7789_INVON 0x21\n");
	ST7789_milli_delay(10);

	ST7789_spi_write_command(ST7789_NORON);	// 6: Normal display on, no args, w/delay: delay(10)
    //PRINTF("ST7789_NORON 0x13\n");
	ST7789_milli_delay(10);

	ST7789_spi_write_command(ST7789_DISPON);	// 7: Main screen turn on, no args, w/delay: delay(500)
    //PRINTF("ST7789_DISPON 0x29\n");
	ST7789_milli_delay(500);
    
    //PRINTF("Display Initialization Done \n");
    return 0;
}

static void ST7789_configure_spi(void) {
    // Configure SPI clock
    uint32_t core_clk = soc_ctrl_peri->SYSTEM_FREQUENCY_HZ;
    uint16_t clk_div = 0;
    if(CLK_MAX_HZ < core_clk/2){
        clk_div = (core_clk/(CLK_MAX_HZ) - 2)/2; // The value is truncated
        if (core_clk/(2 + 2 * clk_div) > CLK_MAX_HZ) clk_div += 1; // Adjust if the truncation was not 0
    }
    //PRINTF("SPI CLK DIV: %d\n", clk_div);
    // SPI Configuration
    // Configure chip 0 (flash memory)
    const uint32_t chip_cfg = spi_create_configopts((spi_configopts_t){
        .clkdiv     = clk_div,
        .csnidle    = 0xF,
        .csntrail   = 0xF,
        .csnlead    = 0xF,
        .fullcyc    = false,
        .cpha       = 0,
        .cpol       = 0
    });

    spi_set_configopts(&ST7789_spi_LCD, 0, chip_cfg);
}

void ST7789_spi_write_command(uint8_t command)
{
    gpio_write(GPIO_SPI_DC, DC_COMMAND);
    spi_write_word(&ST7789_spi_LCD, command);
    spi_wait_for_ready(&ST7789_spi_LCD);
    // Set up segment parameters -> send command and address
    const uint32_t cmd = spi_create_command((spi_command_t){
        .len        = 0,                 // 4 Bytes
        .csaat      = false,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    // Load segment parameters to COMMAND register
    spi_set_command(&ST7789_spi_LCD, cmd);

}

void ST7789_spi_write_data(uint8_t data)
{
    gpio_write(GPIO_SPI_DC, DC_DATA);
    spi_write_word(&ST7789_spi_LCD, data);
    spi_wait_for_ready(&ST7789_spi_LCD);
     // Set up segment parameters -> send command and address
    const uint32_t cmd = spi_create_command((spi_command_t){
        .len        = 0,                 // 4 Bytes
        .csaat      = false,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    // Load segment parameters to COMMAND register
    spi_set_command(&ST7789_spi_LCD, cmd);
}

void ST7789_spi_write_data_2B(uint16_t data)
{
    gpio_write(GPIO_SPI_DC, DC_DATA);
    data = ((data >> 8 & 0x00FF) | (data << 8 & 0xFF00));
    spi_write_word(&ST7789_spi_LCD, data);
    spi_wait_for_ready(&ST7789_spi_LCD);
     // Set up segment parameters -> send command and address
    const uint32_t cmd_read_1 = spi_create_command((spi_command_t){
        .len        = 1,                 // 4 Bytes
        .csaat      = false,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    // Load segment parameters to COMMAND register
    spi_set_command(&ST7789_spi_LCD, cmd_read_1);
}

uint32_t ST7789_test_write_pixel(uint16_t x, uint16_t y, uint16_t color) {
    
    ST7789_set_adress_window(x, y, x+1, y+1);

    ST7789_spi_write_data(color>>8);
    ST7789_spi_write_data(color);

}

void ST7789_test_write_multi_unicolor(uint16_t color, uint32_t num)
{
    while (num > 0)
    {
        ST7789_spi_write_data_2B(color);
        num--;
    }    
}

void ST7789_test_fill_screen(uint16_t color)
{
    ST7789_set_adress_window(0, 0, (uint16_t) ST7789_TFTWIDTH, (uint16_t) ST7789_TFTHEIGHT);
    ST7789_test_write_multi_unicolor(color, (uint32_t) ST7789_TFTWIDTH * ST7789_TFTHEIGHT);
}

void ST7789_fill_picture(uint16_t* colors)
{
    ST7789_set_adress_window(0, 0, (uint16_t) ST7789_TFTWIDTH, (uint16_t) ST7789_TFTHEIGHT);

    int i = 0;
    for ( i = 0; i < (ST7789_TFTWIDTH)/2; i++) {
        for (int repeat_row = 0; repeat_row < 2; repeat_row++) {
            for (int j = 0; j < ST7789_TFTHEIGHT/2; j++) {
                ST7789_spi_write_data_2B(colors[i*ST7789_TFTWIDTH/2+j]);
                ST7789_spi_write_data_2B(colors[i*ST7789_TFTWIDTH/2+j]);
            }
        }
    }
    //PRINTF(" i = %d\n", i);
}

void ST7789_test_fill_picture_with_shift(uint16_t* colors, uint8_t verticalShift, uint8_t horizontalShift)
{
    ST7789_set_adress_window(0, 0, (uint16_t) ST7789_TFTWIDTH, (uint16_t) ST7789_TFTHEIGHT);

    int i = 0;
    for ( i = 0; i < (ST7789_TFTWIDTH)/2; i++) {
        for (int repeat_row = 0; repeat_row < 2; repeat_row++) {
            for (int j = 0; j < ST7789_TFTHEIGHT/2; j++) {
                ST7789_spi_write_data_2B(colors[i*ST7789_TFTWIDTH/2+j+horizontalShift+verticalShift*ST7789_TFTWIDTH/2]);
                ST7789_spi_write_data_2B(colors[i*ST7789_TFTWIDTH/2+j+horizontalShift+verticalShift*ST7789_TFTWIDTH/2]);
            }
        }
    }
}

void ST7789_set_adress_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    // Set column address
    //with driver for screen ST7789V

    ST7789_spi_write_command(ST7789_CASET);

    //(x1<<8 | x1>>8)
    //spi_write_data(x1 >> 8);

    ST7789_spi_write_data_2B(x1);

    //spi_write_data(x2 >> 8);

    ST7789_spi_write_data_2B(x2);

    // Set row address
    ST7789_spi_write_command(ST7789_RASET);

    //spi_write_data(y1 >> 8);

    ST7789_spi_write_data_2B(y1);

    //spi_write_data(y2 >> 8);

    ST7789_spi_write_data_2B(y2);


    // Write to RAM
    ST7789_spi_write_command(0x2C);
}

void ST7789_milli_delay(int n_milli_seconds)
{
    // Converting time into cycles
    //factor found for ZYNQ-Z2 through experimenation
    int cycles = 4*1000 * n_milli_seconds;
 
    for (int i=0; i<cycles; i++) asm volatile("nop;");

}