#include "qemu/osdep.h"
#include "hw/gpio/stm32f2xx_gpio.h"
#include "hw/qdev-properties.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include "migration/vmstate.h"

static void set_pin_cfg(pinObj *to_set, uint8_t pin_index, uint32_t reg_word) {
    uint8_t mode;
    uint8_t cnf;
    mode = (reg_word >> (pin_index * SHAMT)) & GPIOx_MODE_MASK;
    cnf = ((reg_word >> (pin_index * SHAMT)) & GPIOx_CNFy_MASK) >> 2;
    to_set->mode = mode;
    to_set->cnf = cnf;
}

static const VMStateDescription vmstate_stm32f2xx_gpio = {
    .name = TYPE_STM32F2XX_GPIO,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (const VMStateField[]) {
        VMSTATE_UINT32(crl, STM32F2XXGpioState),
        VMSTATE_UINT32(crh, STM32F2XXGpioState),
        VMSTATE_UINT32(idr, STM32F2XXGpioState),
        VMSTATE_UINT32(odr, STM32F2XXGpioState),
        VMSTATE_UINT32(bsrr, STM32F2XXGpioState),
        VMSTATE_UINT32(brr, STM32F2XXGpioState),
        VMSTATE_UINT32(lckr, STM32F2XXGpioState),
        VMSTATE_END_OF_LIST()
    }
};

static uint64_t stm32f2xx_gpio_read(void *opaque, hwaddr addr, unsigned int size) {
    uint64_t to_ret;
    STM32F2XXGpioState *s = STM32F2XX_GPIO(opaque);
    switch(addr) {
        case GPIO_CRL :
            to_ret = s->crl;
            break;
        case GPIO_CRH :
            to_ret = s->crh;
            break;
        case GPIO_IDR :
            to_ret = s->idr;
            break;
        case GPIO_ODR :
            to_ret = s->odr;
            break;
        case GPIO_BSRR :
            to_ret = s->bsrr;
            break;
        case GPIO_BRR :
            to_ret = s->brr;
            break;
        case GPIO_LCKR :
            to_ret = s->lckr;
            break;
        default :
            to_ret = 0x0;
            qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%"HWADDR_PRIx"\n", __func__, addr);
            break;
    }
    return to_ret;
}

/*TODO:
1) model the GPIO in/out configuration
2) IDR register should be read-only because it is written from the outside, and not from the SYSBUS -> [DONE]*/
static void stm32f2xx_gpio_write(void *opaque, hwaddr addr, uint64_t data, unsigned int size) {
    int pin_index;
    STM32F2XXGpioState *s = STM32F2XX_GPIO(opaque);
    switch(addr) {
        /*GPIO ports [0-7] -> LOW*/
        case GPIO_CRL :
            s->crl = data;
            /*Reconfigure PIN LOW status each time CLR is written*/
            for(pin_index = 0; pin_index < GPIOx_NUM_PINS_HALF / 2; pin_index++) {
                set_pin_cfg(&s->pin_state[pin_index], pin_index, s->crl);
            }
            break;
        /*GPIO ports [8-15] -> HIGH*/
        case GPIO_CRH :
            s->crh = data;
            for(pin_index = 0; pin_index < GPIOx_NUM_PINS_HALF; pin_index++) {
                set_pin_cfg(&s->pin_state[pin_index + GPIOx_NUM_PINS_HALF], pin_index, s->crl);
            }
            break;
        case GPIO_IDR :
            /*This register should be READ-ONLY*/
            qemu_log_mask(LOG_UNIMP, "%s: GPIO->IDR is read-only\n", __func__);
            break;
        case GPIO_ODR :
            s->odr = data;
            /*use qemu_set_irq for GPIO pin setting*/
            break;
        case GPIO_BSRR :
            s->bsrr = data;
            break;
        case GPIO_BRR :
            s->brr = data;
            break;
        case GPIO_LCKR :
            s->lckr = data;
            break;
        default :
            qemu_log_mask(LOG_GUEST_ERROR, "%s: Bad offset 0x%"HWADDR_PRIx"\n", __func__, addr);
            break;
    }
}

static const MemoryRegionOps stm32f2xx_gpio_ops = {
    .read = stm32f2xx_gpio_read,
    .write = stm32f2xx_gpio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void stm32f2xx_gpio_reset(DeviceState *dev) {
    STM32F2XXGpioState *s = STM32F2XX_GPIO(dev);
    s->crl = 0x44444444;
    s->crh = 0x44444444;
    s->idr = 0x0;
    s->odr = 0x0;
    s->bsrr = 0x0;
    s->brr = 0x0;
    s->lckr = 0x0;
}


/*Hanlde to GPIO input ports
1) n = pin number
2) level = value of the input pin*/
static void stm32f2xx_gpio_set(void *opaque, int n, int level) {
    STM32F2XXGpioState *gpio_state = STM32F2XX_GPIO(opaque);
    assert(n >= 0 && n < ARRAY_SIZE(gpio_state->irq));
    printf("Parameter N: %d\n", n);
    printf("Parameter LEVEL: %d\n", level);
    printf("Mode of pin %d: %u\n", n, gpio_state->pin_state->mode);
}

static void stm32f2xx_gpio_init(Object *obj) {
    STM32F2XXGpioState *gpio_state = STM32F2XX_GPIO(obj);
    DeviceState *dev = DEVICE(obj);
    memory_region_init_io(&gpio_state->mmio, obj, &stm32f2xx_gpio_ops, gpio_state, TYPE_STM32F2XX_GPIO, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &gpio_state->mmio);
    qdev_init_gpio_in(dev, stm32f2xx_gpio_set, GPIOx_NUM_PINS);
    qdev_init_gpio_out(dev, gpio_state->irq, GPIOx_NUM_PINS);
}

static void stm32f2xx_gpio_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->reset = stm32f2xx_gpio_reset;
    dc->vmsd = &vmstate_stm32f2xx_gpio;
}

static const TypeInfo stm32f2xx_gpio_info = {
    .name = TYPE_STM32F2XX_GPIO,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(STM32F2XXGpioState),
    .instance_init = stm32f2xx_gpio_init,
    .class_init = stm32f2xx_gpio_class_init
};

static void stm32f2xx_gpio_register_types(void)
{
    type_register_static(&stm32f2xx_gpio_info);
}

type_init(stm32f2xx_gpio_register_types)
