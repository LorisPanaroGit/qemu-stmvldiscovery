#ifndef HW_STM32F2XX_RCC_H
#define HW_STM32F2XX_RCC_H

#include "hw/sysbus.h"
#include "qom/object.h"

#define RCC_CR       0x00
#define RCC_CFGR     0x04
#define RCC_CIR      0x08
#define RCC_APB2RSTR 0x0C
#define RCC_APB1RSTR 0x10
#define RCC_AHBENR   0x14
#define RCC_APB2ENR  0x18
#define RCC_APB1ENR  0x1C
#define RCC_BDCR     0x20
#define RCC_CSR      0x24
#define RCC_CFGR2    0x2C

#define TYPE_STM32F2XX_RCC "stm32f2xx-rcc"
OBJECT_DECLARE_SIMPLE_TYPE(STM32F2XXRccState, STM32F2XX_RCC)

struct STM32F2XXRccState {

    SysBusDevice parent_obj;

    MemoryRegion mmio;

    uint32_t rcc_cr;
    uint32_t rcc_cfgr;
    uint32_t rcc_cir;
    uint32_t rcc_apb2rstr;
    uint32_t rcc_apb1rstr;
    uint32_t rcc_ahbenr;
    uint32_t rcc_apb2enr;
    uint32_t rcc_apb1enr;
    uint32_t rcc_bdcr;
    uint32_t rcc_csr;
    uint32_t rcc_cfgr2;

};


#endif