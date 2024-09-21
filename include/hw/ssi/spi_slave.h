#ifndef _SPI_SLAVE_H_
#define _SPI_SLAVE_H_

#include "hw/ssi/ssi.h"
#include "qom/object.h"

#define TYPE_SPI_SLAVE "spi-slave"
OBJECT_DECLARE_SIMPLE_TYPE(SPISLAVEState, SPI_SLAVE)

struct SPISLAVEState {
    SSIPeripheral parent_obj;

    uint8_t val;
    char *master_bus_name;
};


#endif
