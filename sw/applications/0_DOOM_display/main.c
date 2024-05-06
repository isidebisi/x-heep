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
spi_host_t spi_LCD; 



void        ST7789_spi_write_command(uint8_t command);
void        ST7789_spi_write_data(uint8_t data);
void        ST7789_spi_write_data_2B(uint16_t data);







int main(int argc, char *argv[]) {

    PRINTF("START PROGRAM\n");

    ST7789_gpio_init();

    PRINTF("SPI INIT\n");
    

    ST7789_spi_init(&spi_LCD);
    ST7789_display_init();

    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t color = 0xA000;

    //ST7789_test_fill_screen(0x8000);


    PRINTF("PRINT SPI MEM REGION FROM MAIN = %x\n", spi_LCD.base_addr);
    spi_host_t spi_test = ST7789_get_spi_host();
    PRINTF("PRINT SPI MEM REGION FROM DRIVER = %x\n", spi_test.base_addr);

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









void ST7789_spi_write_command(uint8_t command)
{
    gpio_write(GPIO_SPI_DC, DC_COMMAND);
    spi_write_word(&spi_LCD, command);
    PRINTF("SPI HOST ADDRESS = %x\n", spi_LCD.base_addr);
    PRINTF("SPI WRITE COMMAND = %x\n", command);
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

void ST7789_spi_write_data(uint8_t data)
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

void ST7789_spi_write_data_2B(uint16_t data)
{
    gpio_write(GPIO_SPI_DC, DC_DATA);
    data = ((data >> 8 & 0x00FF) | (data << 8 & 0xFF00));
    spi_write_word(&spi_LCD, data);
    //PRINTF("SPI WRITE DATA = %x\n", data);

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

