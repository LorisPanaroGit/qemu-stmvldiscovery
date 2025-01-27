/*
meson test qtest-arm/stm32f2xx_gpio-test -> run single meson test for GPIO
*/

#include "qemu/osdep.h"
#include "libqtest-single.h"

#define GPIO_NUM_PORTS 5
#define GPIO_NUM_REGS  7

#define GPIO_PORT_SIZE 0x400
#define GPIO_REG_SIZE  0x4

#define GPIO_A_ADDR 0x40010800
#define GPIO_B_ADDR 0x40010C00
#define GPIO_C_ADDR 0x40011000
#define GPIO_D_ADDR 0x40011400
#define GPIO_E_ADDR 0x40011800

#define GPIOx_CRL  0x00
#define GPIOx_CRH  0x04
#define GPIOx_IDR  0x08
#define GPIOx_ODR  0x10
#define GPIOx_BSRR 0x10
#define GPIOx_BRR  0x14
#define GPIOx_LCKR 0x18

#define GPIO_PIN_0  0
#define GPIO_PIN_1  1
#define GPIO_PIN_2  2
#define GPIO_PIN_3  3
#define GPIO_PIN_4  4
#define GPIO_PIN_5  5
#define GPIO_PIN_6  6
#define GPIO_PIN_7  7
#define GPIO_PIN_8  8
#define GPIO_PIN_9  9
#define GPIO_PIN_10 10
#define GPIO_PIN_11 11
#define GPIO_PIN_12 12
#define GPIO_PIN_13 13
#define GPIO_PIN_14 14
#define GPIO_PIN_15 15

#define RESET_CRL  0x44444444
#define RESET_CRH  0x44444444
#define RESET_IDR  0x00000000
#define RESET_ODR  0x00000000
#define RESET_BSRR 0x00000000
#define RESET_BRR  0x00000000
#define RESET_LCKR 0x00000000

/* SoC forwards GPIOs to SysCfg */
#define SYSCFG "/machine/soc"

#define PIN_MASK        0xF
#define GPIO_ADDR_MASK  (~(GPIO_PORT_SIZE - 1))

#define GPIO_ADDR(data) ((uintptr_t)(data) & GPIO_ADDR_MASK)
#define GPIO_PIN(data) ((uintptr_t)(data) & PIN_MASK)

unsigned int reset_vals[GPIO_NUM_REGS] = {
    RESET_CRL,
    RESET_CRH,
    RESET_IDR,
    RESET_ODR,
    RESET_BSRR,
    RESET_BRR,
    RESET_LCKR
};

/*test_data() just builds a number by taking the offset of the GPIOx_line and concatenates the pin number*/
static inline void* test_data(uint32_t gpio_line, uint8_t pin_num) {
    return (void*)(uintptr_t) ((gpio_line & GPIO_ADDR_MASK) | (pin_num & PIN_MASK));
}

/*Helper function for writel: pass the GPIO_X group for offset (register address) computing*/
static void gpio_writel(unsigned int gpio_port, unsigned int reg, uint32_t val) {
    writel(gpio_port + reg, val);
}

/*Helper function for readl: pass the GPIO_X group for offset computing*/
static uint32_t gpio_readl(unsigned int gpio_port, unsigned int reg) {
    return readl(gpio_port + reg);
}

/*Helper function to set IRQs on GPIO pins*/
//static void gpio_set_irq(unsigned int gpio_id, int num, int level) {
//    
//}

static void stm32f2xx_system_reset(void) {
    QDict *r;
    r = qtest_qmp(global_qtest, "{'execute': 'system_reset'}");
    g_assert_false(qdict_haskey(r, "error"));
    qobject_unref(r);
}

static void stm32f2xx_test_reset_values(void) {
    unsigned int gpio_addr;
    unsigned int reg_addr;
    unsigned int read_data;
    /*Write unuseful data on every register*/
    for(gpio_addr = GPIO_A_ADDR; gpio_addr <= GPIO_E_ADDR; gpio_addr += GPIO_PORT_SIZE) {
        for(reg_addr = GPIOx_CRL; reg_addr <= GPIOx_LCKR; reg_addr += GPIO_REG_SIZE) {
            gpio_writel(gpio_addr, reg_addr, 0xDEADBEEF);
        }
    }
    stm32f2xx_system_reset();
    for(gpio_addr = GPIO_A_ADDR; gpio_addr <= GPIO_E_ADDR; gpio_addr += GPIO_PORT_SIZE) {
        for(reg_addr = GPIOx_CRL; reg_addr <= GPIOx_LCKR; reg_addr += GPIO_REG_SIZE) {
            read_data = gpio_readl(gpio_addr, reg_addr);
            g_assert_cmphex(read_data, ==, reset_vals[(reg_addr/GPIO_REG_SIZE)]);
        }
    }
}

/*static void stm32f2xx_test_gpio_output_mode(const void *data) {

}*/

static void stm32f2xx_test_gpio_input_mode(const void *data) {
    //uint32_t pin_obj = GPIO_PIN(data);
    //uint32_t gpio_addr = GPIO_ADDR(data);
    //qtest_irq_intercept_in(global_qtest, SYSCFG);
    /*Set GPIOx[GPIO_PIN_<number>] */
}

/*static void stm32f2xx_test_pull_up_pull_down(const void *data) {

}*/

/*static void stm32f2xx_test_push_pull(const void *data) {

}*/

/*static void stm32f2xx_test_open_drain(const void *data) {

}*/

/*static void stm32f2xx_test_bsrr_brr(const void *data) {

}*/

int main(int argc, char **argv) {
    int ret;
    g_test_init(&argc, &argv, NULL);
    g_test_set_nonfatal_assertions();
    qtest_add_func("stm32f2xx/gpio/stm32f2xx_test_reset_values", stm32f2xx_test_reset_values);
    qtest_add_data_func("stm32f2xx/gpio/stm32f2xx_test_input_mode_gpioa", test_data(GPIO_A_ADDR, GPIO_PIN_0), stm32f2xx_test_gpio_input_mode);
    qtest_start("-machine stm32vldiscovery");
    ret = g_test_run();
    qtest_end();

    return ret;
}
