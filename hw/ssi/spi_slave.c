#include "qemu/osdep.h"
#include "hw/ssi/spi_slave.h"
#include "migration/vmstate.h"
#include "qemu/module.h"
#include "hw/qdev-properties.h"

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

static void SPISLAVEState_realize(SSIPeripheral *dev, Error **errp) {
    DeviceState *d = DEVICE(dev);
    BusState *b = d->parent_bus;
    printf("SPI-PERIPHERAL created...\n");
    printf("CS: %u\n", dev->cs_index);
    printf("BUS: %s\n", b->name);
}

static Property SPISLAVEState_prop[] = {
    DEFINE_PROP_STRING("master-bus", SPISLAVEState, master_bus_name),
    DEFINE_PROP_END_OF_LIST()
};

static void SPISLAVEState_reset(DeviceState *dev) {
    SPISLAVEState *s = SPI_SLAVE(dev);
    s->val = 0;
}

static void SPISLAVEState_class_init(ObjectClass *klass, void *data) {
    SSIPeripheralClass *k = SSI_PERIPHERAL_CLASS(klass);
    DeviceClass *dc = DEVICE_CLASS(klass);
    k->realize = SPISLAVEState_realize;
    k->transfer = SPISLAVEState_transfer;
    dc->reset = SPISLAVEState_reset;
    dc->vmsd = &vmstate_spi_slave;
    device_class_set_props(dc, SPISLAVEState_prop);
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
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
