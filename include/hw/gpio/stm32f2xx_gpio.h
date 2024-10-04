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
};

#endif
