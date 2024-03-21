// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "core_v_mini_mcu.h"
#include "gpio.h"
#include "x-heep.h"

#define GPIO_BUTTON 0

/* By default, printfs are activated for FPGA and disabled for simulation. */
#define PRINTF_IN_FPGA  1
#define PRINTF_IN_SIM   0

#if TARGET_SIM && PRINTF_IN_SIM
        #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#elif TARGET_PYNQ_Z2 && PRINTF_IN_FPGA
    #define PRINTF(fmt, ...)    printf(fmt, ## __VA_ARGS__)
#else
    #define PRINTF(...)
#endif


int main(int argc, char *argv[])
{
    gpio_result_t gpio_res;
    gpio_cfg_t pin_cfg = {
        .pin = GPIO_BUTTON,
        .mode = GpioModeIn,
        .en_input_sampling= 0,
        .en_intr = 0
    };
    gpio_res = gpio_config (pin_cfg);
    if (gpio_res != GpioOk)
        PRINTF("Gpio initialization failed!\n");


    bool gpio_val;
    while(1){

        gpio_read(GPIO_BUTTON, &gpio_val); 

        if (gpio_val = 0)
        {
            PRINTF("GPIO = 0 \n\r");
        } else if (gpio_val = 1)
        {
            PRINTF("GPIO = 0 \n\r");
        }
    }

    PRINTF("Success.\n");
    return EXIT_SUCCESS;
}
