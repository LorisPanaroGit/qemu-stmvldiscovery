#include "qemu/osdep.h"
#include "hw/misc/stm32f2xx_rcc.h"
#include "hw/irq.h"
#include "qemu/log.h"


static void stm32f2xx_rcc_reset(DeviceState* dev) {
    STM32F2XXRccState* rcc_state = STM32F2XX_RCC(dev);
    rcc_state->rcc_cr = 0x83;
    rcc_state->rcc_cfgr = 0;
    rcc_state->rcc_cir = 0;
    rcc_state->rcc_apb2rstr = 0;
    rcc_state->rcc_apb1rstr = 0;
    rcc_state->rcc_ahbenr = 0;
    rcc_state->rcc_apb2enr = 0;
    rcc_state->rcc_apb1enr = 0;
    rcc_state->rcc_bdcr = 0;
    rcc_state->rcc_csr = 0;
    rcc_state->rcc_cfgr2 = 0;
}

static uint64_t stm32f2xx_rcc_read(void *opaque, hwaddr adr, unsigned int size) {
    STM32F2XXRccState *rcc_state = STM32F2XX_RCC(opaque);
    uint64_t to_ret;

    switch(adr) {
        case RCC_CR :
            to_ret = rcc_state->rcc_cr;
            break;
        case RCC_CFGR :
            to_ret = rcc_state->rcc_cfgr;
            break;
        case RCC_CIR :
            to_ret = rcc_state->rcc_cir;
            break;
        case RCC_APB2RSTR : 
            to_ret = rcc_state->rcc_apb2rstr;
            break;
        case RCC_APB1RSTR :
            to_ret = rcc_state->rcc_apb1rstr;
            break;
        case RCC_AHBENR :
            to_ret = rcc_state->rcc_ahbenr;
            break;
        case RCC_APB2ENR :
            to_ret = rcc_state->rcc_apb2enr;
            break;
        case RCC_APB1ENR :
            to_ret = rcc_state->rcc_apb1enr;
            break;
        case RCC_BDCR :
            to_ret = rcc_state->rcc_bdcr;
            break;
        case RCC_CSR :
            to_ret = rcc_state->rcc_csr;
            break;
        case RCC_CFGR2 :
            to_ret = rcc_state->rcc_cfgr2;
            break;
        default :
            to_ret = 0x0;
            qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%"HWADDR_PRIx"\n", __func__, adr);
            break;
    }
    return to_ret;
}

static void stm32f2xx_rcc_write(void *opaque, hwaddr adr, uint64_t val, unsigned int size) {
    STM32F2XXRccState *rcc_state = STM32F2XX_RCC(opaque);
    uint32_t val_to_write = val & 0xFFFFFFFF;

    switch(adr) {
        case RCC_CR :
            rcc_state->rcc_cr = val_to_write;
            break;
        case RCC_CFGR :
            rcc_state->rcc_cfgr = val_to_write;
            break;
        case RCC_CIR :
            rcc_state->rcc_cir = val_to_write;
            break;
        case RCC_APB2RSTR :
            rcc_state->rcc_apb2rstr = val_to_write;
            break;
        case RCC_APB1RSTR :
            rcc_state->rcc_apb1rstr = val_to_write;
            break;
        case RCC_AHBENR :
            rcc_state->rcc_ahbenr = val_to_write;
            break;
        case RCC_APB2ENR :
            rcc_state->rcc_apb2enr = val_to_write;
            break;
        case RCC_APB1ENR :
            rcc_state->rcc_apb1enr = val_to_write;
            break;
        case RCC_BDCR :
            rcc_state->rcc_bdcr = val_to_write;
            break;
        case RCC_CSR :
            rcc_state->rcc_csr = val_to_write;
            break;
        case RCC_CFGR2 :
            rcc_state->rcc_cfgr2 = val_to_write;
            break;
        default :
            qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%"HWADDR_PRIx"\n", __func__, adr);
            break;
    }
}

static const MemoryRegionOps stm32f2xx_rcc_ops = {
    .read = stm32f2xx_rcc_read,
    .write = stm32f2xx_rcc_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

//static Property stm32f2xx_rcc_properties[] = {
//    
//}

static void stm32f2xx_rcc_init(Object *obj) {
    STM32F2XXRccState *rcc_state = STM32F2XX_RCC(obj);
    memory_region_init_io(&rcc_state->mmio, obj, &stm32f2xx_rcc_ops, rcc_state, TYPE_STM32F2XX_RCC, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &rcc_state->mmio);
}

static void stm32f2xx_rcc_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    device_class_set_legacy_reset(dc, stm32f2xx_rcc_reset);
}

static const TypeInfo stm32f2xx_rcc_info = {
    .name          = TYPE_STM32F2XX_RCC,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(STM32F2XXRccState),
    .instance_init = stm32f2xx_rcc_init,
    .class_init    = stm32f2xx_rcc_class_init,
};

static void stm32f2xx_rcc_register_types(void)
{
    type_register_static(&stm32f2xx_rcc_info);
}

type_init(stm32f2xx_rcc_register_types)

