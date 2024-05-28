#include "x_spi.h"

#include "x-heep.h"
#include "w25q128jw.h"

spi_host_t spi_flash;


void X_init_spi()
{
    spi_flash.base_addr = mmio_region_from_addr((uintptr_t)SPI_HOST_START_ADDRESS);
    w25q128jw_init(spi_flash);
}

void X_spi_read(uint32_t address, uint32_t *data, uint32_t len)
{
    w25q128jw_read_standard(address, data, len);
}