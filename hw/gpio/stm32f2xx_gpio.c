#include "qemu/osdep.h"
#include "hw/gpio/stm32f2xx_gpio.h"
#include "hw/qdev-properties.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/irq.h"
#include "migration/vmstate.h"
#include "trace.h"

static void set_pin_cfg(pinObj *to_set, uint8_t pin_index, uint32_t reg_word) {
    uint8_t mode;
    uint8_t cnf;
    mode = (reg_word >> (pin_index * SHAMT)) & GPIOx_MODE_MASK;
    cnf = ((reg_word >> (pin_index * SHAMT)) & GPIOx_CNFy_MASK) >> 2;
    to_set->mode = mode;
    to_set->cnf = cnf;
}

/*If MODEx == 0: pin is input
If MODEx > 0: pin is output*/
static bool is_output(STM32F2XXGpioState *gpio_state, int pin_index) {
    return gpio_state->pin_state[pin_index].mode > 0;
}

/*If CNFx == 0: output is PUSH-PULL
If CNFx == 1: output is OPEN-DRAIN*/
static bool is_push_pull(STM32F2XXGpioState *gpio_state, int pin_index) {
    return gpio_state->pin_state[pin_index].cnf == PUSH_PULL_OUTPUT;
}

static bool is_open_drain(STM32F2XXGpioState *gpio_state, int pin_index) {
    return gpio_state->pin_state[pin_index].cnf == OPEN_DRAIN_OUTPUT;
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

/*Whenever ODR is written, stm32f2xx_gpio_set_irq() will trigger and interrupt on the @n pin*/
static void stm32f2xx_gpio_set_irq(STM32F2XXGpioState *gpio_state, int n, int level) {
    /*OPEN-DRAIN case: if @level is 1, the line cannot be set; if @level is 0, then it can be driven LOW*/
    /*is_output() check is already done at higher level*/
    if(is_push_pull(gpio_state, n)) {
        qemu_set_irq(gpio_state->irq[n], level);
        printf("Pin number: %d\n", n);
        printf("Level: %d\n", level);
    } else if(is_open_drain(gpio_state, n) && level == 0) {
        /*OPEN-DRAIN case: if @level is 1, the line cannot be set; if @level is 0, then it can be driven LOW*/
        qemu_set_irq(gpio_state->irq[n], 0);
    } else {
        qemu_log_mask(LOG_GUEST_ERROR, "Line %d is OPEN-DRAIN: cannot be set to 1\n", n);
    }
    trace_stm32f2xx_gpio_update_output_irq(n, level);
}

static void stm32f2xx_gpio_config_pins(STM32F2XXGpioState *gpio_state, int start, int end, uint32_t reg) {
    int pin_index;
    for(pin_index = start; pin_index < end; pin_index++) {
        /*pin index - start: shift towards 0 the pin index of pins >= 8*/
        set_pin_cfg(&gpio_state->pin_state[pin_index], pin_index - start, reg);
    }
}

static void stm32f2xx_gpio_config_output_irqs(STM32F2XXGpioState *gpio_state) {
    int pin_index;
    int level;
    for(pin_index = 0; pin_index < GPIOx_NUM_PINS; pin_index++) {
        if(is_output(gpio_state, pin_index)) {
            level = extract32(gpio_state->odr, pin_index, 1);
            stm32f2xx_gpio_set_irq(gpio_state, pin_index, level);
        }
    }
}

/*Handle to GPIO input ports
1) n = pin number
2) level = value of the input pin*/
static void stm32f2xx_gpio_set(void *opaque, int n, int level) {
    STM32F2XXGpioState *gpio_state = STM32F2XX_GPIO(opaque);
    assert(n >= 0 && n < ARRAY_SIZE(gpio_state->irq));
    unsigned short idr_mask = (0x1 << n);
    if(is_output(gpio_state, n)) {
        qemu_log_mask(LOG_GUEST_ERROR, "Line %d can't be driven externally\n", n);
    } else {
        gpio_state->idr = (level) ? (gpio_state->idr | idr_mask) : (gpio_state->idr &~ idr_mask);
    }
    trace_stm32f2xx_gpio_update_input_irq(n, level);
}

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
            qemu_log_mask(LOG_GUEST_ERROR, "%s: GPIO->BSRR is write-only\n", __func__);
            to_ret = 0x0;
            break;
        case GPIO_BRR :
            qemu_log_mask(LOG_GUEST_ERROR, "%s: GPIO->BRR is write-only\n", __func__);
            to_ret = 0x0;
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
1) model LCKR behaviour*/
static void stm32f2xx_gpio_write(void *opaque, hwaddr addr, uint64_t data, unsigned int size) {
    uint32_t bits_to_set;
    uint32_t bits_to_reset;
    STM32F2XXGpioState *s = STM32F2XX_GPIO(opaque);
    switch(addr) {
        /*GPIO ports [0-7] -> LOW*/
        case GPIO_CRL :
            s->crl = data;
            /*Reconfigure PIN LOW status each time CLR is written*/
            stm32f2xx_gpio_config_pins(s, 0, GPIOx_NUM_PINS_HALF, s->crl);
            break;
        /*GPIO ports [8-15] -> HIGH*/
        case GPIO_CRH :
            s->crh = data;
            stm32f2xx_gpio_config_pins(s, GPIOx_NUM_PINS_HALF, GPIOx_NUM_PINS, s->crh);
            break;
        case GPIO_IDR :
            /*This register should be READ-ONLY*/
            qemu_log_mask(LOG_GUEST_ERROR, "%s: GPIO->IDR is read-only\n", __func__);
            break;
        case GPIO_ODR :
            /*Before writing, mask @data 0xFFFF to reset reserved bits [16:31] of ODR*/
            s->odr = data & 0xFFFF;
            /*use qemu_set_irq for GPIO pin setting*/
            stm32f2xx_gpio_config_output_irqs(s);
            break;
        case GPIO_BSRR :
            /*For atomic bit set/reset, the ODR bits can be individually set and cleared by writing to
            the GPIOx_BSRR register (x = A .. E).*/
            s->bsrr = data;
            bits_to_reset = extract32(s->bsrr, GPIOx_NUM_PINS, GPIOx_NUM_PINS);
            bits_to_set = extract32(s->bsrr, 0, GPIOx_NUM_PINS);
            /*0: No action on the corresponding ODRx bit
            1: Reset the corresponding ODRx bit*/
            s->odr &= ~bits_to_reset;
            s->odr |= bits_to_set;
            /*use qemu_set_irq for GPIO pin setting*/
            stm32f2xx_gpio_config_output_irqs(s);
            break;
        case GPIO_BRR :
            /*Before writing, mask @data 0xFFFF to reset reserved bits [16:31] of BRR*/
            s->brr = data & 0xFFFF;
            /*0: No action on the corresponding ODRx bit
            1: Reset the corresponding ODRx bit*/
            s->odr &= ~s->brr;
            stm32f2xx_gpio_config_output_irqs(s);
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

static void stm32f2xx_gpio_init(Object *obj) {
    STM32F2XXGpioState *gpio_state = STM32F2XX_GPIO(obj);
    DeviceState *dev = DEVICE(obj);
    memory_region_init_io(&gpio_state->mmio, obj, &stm32f2xx_gpio_ops, gpio_state, TYPE_STM32F2XX_GPIO, 0x400);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &gpio_state->mmio);
    qdev_init_gpio_in(dev, stm32f2xx_gpio_set, GPIOx_NUM_PINS);
    qdev_init_gpio_out(dev, gpio_state->irq, GPIOx_NUM_PINS);
}

static void stm32f2xx_gpio_class_init(ObjectClass *klass, const void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    device_class_set_legacy_reset(dc, stm32f2xx_gpio_reset);
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
