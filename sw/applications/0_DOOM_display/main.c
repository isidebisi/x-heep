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

/* To get TX and RX FIFO depth */
#include "spi_host_regs.h"

/* To get the target of the compilation (sim or pynq) */
#include "x-heep.h"

/* To get the soc_ctrl base address */
#include "soc_ctrl_structs.h"
#include "spi_host.h"

/* get the GPIO library*/
#include "gpio.h" 

#include "main.h"

#include "core_v_mini_mcu.h"

#define CLK_MAX_HZ (133*1000*1000)

#define GPIO_SPI_DC 8
#define DC_COMMAND 0
#define DC_DATA 1

#define GPIO_SPI_RST 13 //AR2

#define GPIO_SPI_CS 14 //AR3
#define CS_SELECT 0
#define CS_DESELECT 1

/**
 * @brief SPI structure.
*/
extern spi_host_t __attribute__((section(".xheep_init_data_crt0"))) spi; //this variable is also used by the crt0, thus keep it in this section


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

#ifdef TARGET_PYNQ_Z2
    #define USE_SPI_FLASH
#endif

//Test functions
uint8_t spi_init(void);
uint8_t display_init(void);
static void configure_spi(void);
void spi_write_command(uint32_t command);
void spi_write_data(uint32_t data);

void set_adress_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
uint32_t test_write_pixel(uint16_t x, uint16_t y, uint16_t color);
void milli_delay(int n_milli_seconds);





int main(int argc, char *argv[]) {

    gpio_cfg_t cfg_out = {
        .pin = GPIO_SPI_DC,
        .mode = GpioModeOutPushPull
    };

    gpio_cfg_t cfg_out2 = {
        .pin = GPIO_SPI_RST,
        .mode = GpioModeOutPushPull
    };

    gpio_cfg_t cfg_out3 = {
        .pin = GPIO_SPI_CS,
        .mode = GpioModeOutPushPull
    };
    gpio_write(GPIO_SPI_CS, CS_DESELECT);



    spi.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);

    spi_init();
    display_init();

    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t color = 0x0413;

    while(1){
        PRINTF("RESTART with color: %d\n", color);
        color += 0x8888;
        x = 30;
        y = 30;

        while (x<240)
        {
            PRINTF("x: %d\n", x);

            while (y<240)
            {
                PRINTF("y: %d\n", y);
                test_write_pixel(x, y, color);
                y++;
            }
            y=30;
            x++;
        }
    }

    
    

}

uint8_t spi_init(void) {

        // Enable SPI host device
    spi_set_enable(&spi, true);

    // Enable SPI output
    spi_output_enable(&spi, true);

    // Configure SPI connection on CSID 0
    configure_spi();

    // Set CSID
    spi_set_csid(&spi, 0);

    return 0;

}

uint8_t display_init(void)
{
    //display reset
    PRINTF("Display Reset\n");
    gpio_write(GPIO_SPI_RST, 1);
    milli_delay(100);
    gpio_write(GPIO_SPI_RST, 0);
    milli_delay(100);
    gpio_write(GPIO_SPI_RST, 1);
    milli_delay(100);

    spi_write_command(ST7789_SWRESET);	//1: Software reset, no args, w/delay: delay(150)
    milli_delay(150);
	
    spi_write_command(ST7789_SLPOUT);	// 2: Out of sleep mode, no args, w/delay: delay(500)
	milli_delay(500);

	spi_write_command(ST7789_COLMOD);	// 3: Set color mode, 1 arg, delay: delay(10)
	milli_delay(10);

    spi_write_data(ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT);	// 65K color, 16-bit color
	milli_delay(150);

    spi_write_command(ST7789_MADCTL);	// 4: Memory access ctrl (directions), 1 arg:
	spi_write_data(ST7789_MADCTL_RGB);	// RGB Color

    spi_write_command(ST7789_INVON);	// 5: Inversion ON (but why?) delay(10)
	milli_delay(10);

	spi_write_command(ST7789_NORON);	// 6: Normal display on, no args, w/delay: delay(10)
	milli_delay(10);

	spi_write_command(ST7789_DISPON);	// 7: Main screen turn on, no args, w/delay: delay(500)
	milli_delay(500);
    
    PRINTF("Display Reset Done\n");
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

    spi_set_configopts(&spi, 0, chip_cfg);
}

void spi_write_command(uint32_t command)
{
    gpio_write(GPIO_SPI_CS, CS_SELECT);
    gpio_write(GPIO_SPI_DC, DC_COMMAND);
    spi_write_word(&spi, command);
    milli_delay(5);
    gpio_write(GPIO_SPI_CS, CS_DESELECT);
}

void spi_write_data(uint32_t data)
{
    gpio_write(GPIO_SPI_CS, CS_SELECT);
    gpio_write(GPIO_SPI_DC, DC_DATA);
    spi_write_word(&spi, data);
    milli_delay(5);
    gpio_write(GPIO_SPI_CS, CS_DESELECT);
}

uint32_t test_write_pixel(uint16_t x, uint16_t y, uint16_t color) {
    
    set_adress_window(x, y, x, y);

    spi_write_data(color);

}

void set_adress_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    // Set column address
    //with driver for screen ST7789V

    spi_write_command(0x2A);
    spi_wait_for_ready(&spi);

    spi_write_data(x1 >> 8);
    spi_wait_for_ready(&spi);

    spi_write_data(x1 & 0xFF);
    spi_wait_for_ready(&spi);

    spi_write_data(x2 >> 8);
    spi_wait_for_ready(&spi);

    spi_write_data(x2 & 0xFF);
    spi_wait_for_ready(&spi);

    // Set row address
    spi_write_command(0x2B);
    spi_wait_for_ready(&spi);

    spi_write_data(y1 >> 8);
    spi_wait_for_ready(&spi);

    spi_write_data(y1 & 0xFF);
    spi_wait_for_ready(&spi);

    spi_write_data(y2 >> 8);
    spi_wait_for_ready(&spi);

    spi_write_data(y2 & 0xFF);
    spi_wait_for_ready(&spi);


    // Write to RAM
    spi_write_command(0x2C);
    spi_wait_for_ready(&spi);
}

void milli_delay(int n_milli_seconds)
{
    // Converting time into cycles
    //factor found for ZYNQ-Z2 through experimenation
    int cycles = 4*1000 * n_milli_seconds;
 
    for (int i=0; i<cycles; i++) asm volatile("nop;");

}