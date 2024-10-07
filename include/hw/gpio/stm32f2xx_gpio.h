#ifndef HW_STM32F2XX_GPIO_H
#define HW_STM32F2XX_GPIO_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define TYPE_STM32F2XX_GPIO "stm32f2xx-gpio"
OBJECT_DECLARE_SIMPLE_TYPE(STM32F2XXGpioState, STM32F2XX_GPIO)

#define GPIO_CRL  0x0
#define GPIO_CRH  0x4
#define GPIO_IDR  0x8
#define GPIO_ODR  0xC
#define GPIO_BSRR 0x10
#define GPIO_BRR  0x14
#define GPIO_LCKR 0x18

#define GPIOx_NUM_PINS      16
#define GPIOx_NUM_PINS_HALF 8

/*Masks defines*/
#define SHAMT 4
#define GPIOx_MODE_MASK 0b11
#define GPIOx_CNFy_MASK 0b1100

/*INPUT MODE defines*/
#define ANALOG_INPUT   0b00
#define FLOAT_INPUT    0b01
#define PUP_PUD_INPUT  0b10 /*PULL-UP / PULL-DOWN : digital behaviour of the pin*/
#define RESERVED_INPUT 0b11

#define PUSH_PULL_OUTPUT  0b00
#define OPEN_DRAIN_OUTPUT 0b01

typedef struct pinObj {
    unsigned char mode;
    unsigned char cnf;
} pinObj;

struct STM32F2XXGpioState {
    
    SysBusDevice parent_obj;

    MemoryRegion mmio;

    /*GPIO registers*/
    uint32_t crl;
    uint32_t crh;
    uint32_t idr;
    uint32_t odr;
    uint32_t bsrr;
    uint32_t brr;
    uint32_t lckr;

    /*GPIO pin helper struct*/
    pinObj pin_state[GPIOx_NUM_PINS];

    /*QEMU GPIO pins model using qemu_irq -> cannot stay inside pinObj struct because qdev_init_gpio_out() takes qemu_irq pointer and array size*/
    qemu_irq irq[GPIOx_NUM_PINS];
};

#endif
