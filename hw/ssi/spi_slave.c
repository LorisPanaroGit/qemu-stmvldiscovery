/*
    Removing spi_slave.h header from file system
    spi-slave will not be embedded in any board/device so it is redundant
*/

#include "qemu/osdep.h"
#include "migration/vmstate.h"
#include "qemu/module.h"
#include "hw/qdev-properties.h"

#include "hw/ssi/ssi.h"
#include "qom/object.h"

#define TYPE_SPI_SLAVE "spi-slave"
OBJECT_DECLARE_SIMPLE_TYPE(SPISLAVEState, SPI_SLAVE)

struct SPISLAVEState {
    SSIPeripheral parent_obj;
    uint8_t val;
};


static uint32_t SPISLAVEState_transfer(SSIPeripheral *spi, uint32_t value) {
    SPISLAVEState *s = SPI_SLAVE(spi);
    s->val = value;
    printf("%c", s->val);
    return s->val;
}

static const VMStateDescription vmstate_spi_slave = {
    .name = TYPE_SPI_SLAVE,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (const VMStateField[]) {
        VMSTATE_SSI_PERIPHERAL(parent_obj, SPISLAVEState),
        VMSTATE_UINT8(val, SPISLAVEState),
        VMSTATE_END_OF_LIST()
    }
};

static void SPISLAVEState_realize(SSIPeripheral *spi_slave, Error **errp) {
    DeviceState *dev = DEVICE(spi_slave);
    printf("%s created...\n", dev->id); 
}

static void SPISLAVEState_reset(DeviceState *dev) {
    SPISLAVEState *s = SPI_SLAVE(dev);
    s->val = 0;
}

static void SPISLAVEState_class_init(ObjectClass *klass, void *data) {
    SSIPeripheralClass *k = SSI_PERIPHERAL_CLASS(klass);
    DeviceClass *dc = DEVICE_CLASS(klass);
    k->realize = SPISLAVEState_realize;
    k->transfer = SPISLAVEState_transfer;
    device_class_set_legacy_reset(dc, SPISLAVEState_reset);
    dc->vmsd = &vmstate_spi_slave;
}

static const TypeInfo SPISLAVEState_info = {
    .name = TYPE_SPI_SLAVE,
    .parent = TYPE_SSI_PERIPHERAL,
    .instance_size = sizeof(SPISLAVEState),
    .class_init = SPISLAVEState_class_init,
    .abstract = false
};

static void SPISLAVEState_register_types(void) {
    type_register_static(&SPISLAVEState_info);
}

type_init(SPISLAVEState_register_types)
