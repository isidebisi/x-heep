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
// AR2 = RST
// AR3 = CS
// AR4 = SCLK
// AR8 = MOSI




/* By default, PRINTFs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif

#define TARGET_PYNQ_Z2 1


//Test functions
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

/**
 * @brief SPI structure.
*/
 spi_host_t spi_LCD; //this variable is also used by the crt0, thus keep it in this section



int main(int argc, char *argv[]) {

    PRINTF("START PROGRAM V2\n");

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


    PRINTF("SPI INIT\n");
    spi_LCD.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);

    spi_init();
    display_init();

    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t color = 0xA000;

    test_fill_screen(0x89AB);

    while(1)
    {
        fill_picture(&filtered_array);
        //test_fill_screen(color);
        //PRINTF("Fill with color: %d\n", color);
        //color += 0x0003;
        PRINTF("LOOP FINISHED\n");
        milli_delay(500);
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
                test_write_pixel(x, y, color);
                milli_delay(1);
                y++;
            }
            y=30;
            x++;
        }
    }

    
    

}

uint8_t spi_init(void) {

        // Enable SPI host device
    spi_set_enable(&spi_LCD, true);

    // Enable SPI output
    spi_output_enable(&spi_LCD, true);

    // Configure SPI connection on CSID 0
    configure_spi();

    // Set CSID
    spi_set_csid(&spi_LCD, 0);
    milli_delay(100);

    return 0;

}

uint8_t display_init(void)
{
    //display reset
    PRINTF("Display Initialization \n");
    gpio_write(GPIO_SPI_RST, 1);
    milli_delay(100);
    gpio_write(GPIO_SPI_RST, 0);
    milli_delay(100);
    gpio_write(GPIO_SPI_RST, 1);
    milli_delay(100);

    gpio_write(GPIO_SPI_DC, DC_COMMAND);

    spi_write_command(ST7789_SWRESET);	//1: Software reset, no args, w/delay: delay(150)
    PRINTF("ST7789_SWRESET 0x01\n");
    milli_delay(150);
	//milli_delay(2000);

    spi_write_command(ST7789_SLPOUT);	// 2: Out of sleep mode, no args, w/delay: delay(500)
    PRINTF("ST7789_SLPOUT 0x11\n");
	milli_delay(500);
    //milli_delay(2000);

	spi_write_command(ST7789_COLMOD);	// 3: Set color mode, 1 arg, delay: delay(10)
    PRINTF("ST7789_COLMOD 0x3A\n");

    spi_write_data(ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT);	// 65K color, 16-bit color
    PRINTF("ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT = 0x55\n");
	milli_delay(150);
    //milli_delay(2000);

    spi_write_command(ST7789_MADCTL);	// 4: Memory access ctrl (directions), 1 arg:
    PRINTF("ST7789_MADCTL 0x36\n");

	spi_write_data(ST7789_MADCTL_RGB);	// RGB Color
    PRINTF("ST7789_MADCTL_RGB = 0x00\n");

    spi_write_command(ST7789_CASET);	// 5: Column addr set, 4 args, no delay:
    PRINTF("ST7789_CASET 0x2A\n");
    spi_write_data(ST7789_240x240_XSTART >> 8);
    spi_write_data(ST7789_240x240_XSTART);
    spi_write_data((ST7789_TFTWIDTH + ST7789_240x240_XSTART) >> 8);
    spi_write_data((ST7789_TFTWIDTH + ST7789_240x240_XSTART));

    spi_write_command(ST7789_RASET);	// 6: Row addr set, 4 args, no delay:
    PRINTF("ST7789_RASET 0x2B\n");
    spi_write_data(ST7789_240x240_YSTART >> 8);
    spi_write_data(ST7789_240x240_YSTART);
    spi_write_data((ST7789_TFTHEIGHT + ST7789_240x240_YSTART) >> 8);
    spi_write_data((ST7789_TFTHEIGHT + ST7789_240x240_YSTART));

    //milli_delay(2000);
    spi_write_command(ST7789_INVON);	// 5: Inversion ON (but why?) delay(10)
    PRINTF("ST7789_INVON 0x21\n");
	milli_delay(10);
    //milli_delay(2000);

	spi_write_command(ST7789_NORON);	// 6: Normal display on, no args, w/delay: delay(10)
    PRINTF("ST7789_NORON 0x13\n");
	milli_delay(10);
    //milli_delay(2000);

	spi_write_command(ST7789_DISPON);	// 7: Main screen turn on, no args, w/delay: delay(500)
    PRINTF("ST7789_DISPON 0x29\n");
	milli_delay(500);
    
    PRINTF("Display Initialization Done \n");
    return 0;
}

static void configure_spi(void) {
    // Configure SPI clock
    uint32_t core_clk = soc_ctrl_peri->SYSTEM_FREQUENCY_HZ;
    uint16_t clk_div = 0;
    if(CLK_MAX_HZ < core_clk/2){
        clk_div = (core_clk/(CLK_MAX_HZ) - 2)/2; // The value is truncated
        if (core_clk/(2 + 2 * clk_div) > CLK_MAX_HZ) clk_div += 1; // Adjust if the truncation was not 0
    }
    PRINTF("SPI CLK DIV: %d\n", clk_div);
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

    spi_set_configopts(&spi_LCD, 0, chip_cfg);
}

void spi_write_command(uint8_t command)
{
    gpio_write(GPIO_SPI_DC, DC_COMMAND);
    spi_write_word(&spi_LCD, command);
    spi_wait_for_ready(&spi_LCD);
    // Set up segment parameters -> send command and address
    const uint32_t cmd = spi_create_command((spi_command_t){
        .len        = 0,                 // 4 Bytes
        .csaat      = false,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    // Load segment parameters to COMMAND register
    spi_set_command(&spi_LCD, cmd);

}

void spi_write_data(uint8_t data)
{
    gpio_write(GPIO_SPI_DC, DC_DATA);
    spi_write_word(&spi_LCD, data);
    spi_wait_for_ready(&spi_LCD);
     // Set up segment parameters -> send command and address
    const uint32_t cmd = spi_create_command((spi_command_t){
        .len        = 0,                 // 4 Bytes
        .csaat      = false,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    // Load segment parameters to COMMAND register
    spi_set_command(&spi_LCD, cmd);
}

void spi_write_data_2B(uint16_t data)
{
    gpio_write(GPIO_SPI_DC, DC_DATA);
    data = ((data >> 8 & 0x00FF) | (data << 8 & 0xFF00));
    spi_write_word(&spi_LCD, data);
    spi_wait_for_ready(&spi_LCD);
     // Set up segment parameters -> send command and address
    const uint32_t cmd_read_1 = spi_create_command((spi_command_t){
        .len        = 1,                 // 4 Bytes
        .csaat      = false,              // Command not finished
        .speed      = kSpiSpeedStandard, // Single speed
        .direction  = kSpiDirTxOnly      // Write only
    });
    // Load segment parameters to COMMAND register
    spi_set_command(&spi_LCD, cmd_read_1);
}

uint32_t test_write_pixel(uint16_t x, uint16_t y, uint16_t color) {
    
    set_adress_window(x, y, x+1, y+1);

    spi_write_data(color>>8);
    spi_write_data(color);

}

void test_write_multi_unicolor(uint16_t color, uint32_t num)
{
    while (num > 0)
    {
        spi_write_data_2B(color);
        num--;
    }    
}

void test_fill_screen(uint16_t color)
{
    set_adress_window(0, 0, (uint16_t) ST7789_TFTWIDTH, (uint16_t) ST7789_TFTHEIGHT);
    test_write_multi_unicolor(color, (uint32_t) ST7789_TFTWIDTH * ST7789_TFTHEIGHT);
}

void fill_picture(uint16_t* colors)
{
    set_adress_window(0, 0, (uint16_t) ST7789_TFTWIDTH, (uint16_t) ST7789_TFTHEIGHT);

    int i = 0;
    for ( i = 0; i < (ST7789_TFTWIDTH)/2; i++) {
        for (int repeat_row = 0; repeat_row < 2; repeat_row++) {
            for (int j = 0; j < ST7789_TFTHEIGHT/2; j++) {
                spi_write_data_2B(colors[i*ST7789_TFTWIDTH/2+j]);
                spi_write_data_2B(colors[i*ST7789_TFTWIDTH/2+j]);
            }
        }
    }
    PRINTF(" i = %d\n", i);
}


void set_adress_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    // Set column address
    //with driver for screen ST7789V

    spi_write_command(ST7789_CASET);

    //(x1<<8 | x1>>8)
    //spi_write_data(x1 >> 8);

    spi_write_data_2B(x1);

    //spi_write_data(x2 >> 8);

    spi_write_data_2B(x2);

    // Set row address
    spi_write_command(ST7789_RASET);

    //spi_write_data(y1 >> 8);

    spi_write_data_2B(y1);

    //spi_write_data(y2 >> 8);

    spi_write_data_2B(y2);


    // Write to RAM
    spi_write_command(0x2C);
}

void milli_delay(int n_milli_seconds)
{
    // Converting time into cycles
    //factor found for ZYNQ-Z2 through experimenation
    int cycles = 4*1000 * n_milli_seconds;
 
    for (int i=0; i<cycles; i++) asm volatile("nop;");

}