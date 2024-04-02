// Copyright EPFL contributors.
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0

#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "gpio.h"
#include "pad_control.h"
#include "pad_control_regs.h"  // Generated.
#include "x-heep.h"
#include <limits.h> //todo: remove



/* By default, printfs are activated for FPGA and disabled for simulation. */
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


#ifndef RV_PLIC_IS_INCLUDED
  #error ( "This app does NOT work as the RV_PLIC peripheral is not included" )
#endif


#define GPIO_TB_IN  9
#define GPIO_INTR  GPIO_INTR_9

plic_result_t plic_res;

uint8_t gpio_intr_flag = 0;

void handler_1()
{
    PRINTF("handler 1\n");
    gpio_intr_flag = 1;
}

void handler_2()
{
    PRINTF("handler 2\n");
    gpio_intr_flag = 1;
}

int main(int argc, char *argv[])
{

    pad_control_t pad_control;
    pad_control.base_addr = mmio_region_from_addr((uintptr_t)PAD_CONTROL_START_ADDRESS);
    // rv_plic_params.base_addr = mmio_region_from_addr((uintptr_t)RV_PLIC_START_ADDRESS);
    plic_res = plic_Init();
    if (plic_res != kPlicOk) {
        PRINTF("Init PLIC failed\n\r;");
        return -1;
    }


    plic_res = plic_irq_set_priority(GPIO_INTR, 1);
    if (plic_res != kPlicOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }

    plic_res = plic_irq_set_enabled(GPIO_INTR, kPlicToggleEnabled);
    if (plic_res != kPlicOk) {
        PRINTF("Failed\n\r;");
        return -1;
    }

    // Enable interrupt on processor side
    // Enable global interrupt for machine-level interrupts
    CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    // Set mie.MEIE bit to one to enable machine-level external interrupts
    const uint32_t mask = 1 << 11;
    CSR_SET_BITS(CSR_REG_MIE, mask);

    //gpio_reset_all();
    gpio_result_t gpio_res;


    gpio_cfg_t cfg_in = {
        .pin = GPIO_TB_IN,
        .mode = GpioModeIn,
        .en_input_sampling = true,
        .en_intr = true,
        .intr_type = GpioIntrEdgeFalling
    };
    gpio_res = gpio_config(cfg_in);
    if (gpio_res != GpioOk) {
        PRINTF("Failed\n;");
        return -1;
    }

    gpio_assign_irq_handler( GPIO_INTR, &handler_1 );
    gpio_intr_flag = 0;

    PRINTF("Press Button for interrupt...\n\r");
    while(gpio_intr_flag == 0) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        // wait_for_interrupt();GPIO
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
        PRINTF("Wait for button in PIN AR1 press for interrupt...\n\r");
    }

    gpio_assign_irq_handler( GPIO_INTR, &handler_2 );
    gpio_intr_flag = 0;

    PRINTF("Press Button for interrupt...\n\r");
    while(gpio_intr_flag == 0) {
        // disable_interrupts
        // this does not prevent waking up the core as this is controlled by the MIP register
        CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);
        //wait_for_interrupt();
        CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);
    }

    PRINTF("Success\n\r");

    return EXIT_SUCCESS;
}
