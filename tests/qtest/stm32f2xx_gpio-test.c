/*
meson test qtest-arm/stm32f2xx_gpio-test -> run single meson test for GPIO
*/

#include "qemu/osdep.h"
#include "libqtest-single.h"

#define HIGH 1
#define LOW 0

#define GPIO_NUM_PORTS 5
#define GPIO_NUM_REGS  7

#define GPIO_PORT_SIZE 0x400
#define GPIO_REG_SIZE  0x4
#define GPIO_BASE_ADDR 0x40010800

#define GPIO_A_GROUP 0
#define GPIO_B_GROUP 1
#define GPIO_C_GROUP 2
#define GPIO_D_GROUP 3
#define GPIO_E_GROUP 4

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

#define GPIO_NUM_PINS 16

#define RESET_CRL  0x44444444
#define RESET_CRH  0x44444444
#define RESET_IDR  0x00000000
#define RESET_ODR  0x00000000
#define RESET_BSRR 0x00000000
#define RESET_BRR  0x00000000
#define RESET_LCKR 0x00000000

#define GPIOx_CNF_INPUT 0x0
#define GPIOx_MODE_INPUT 0x0

/* SoC forwards GPIOs to SysCfg */
#define SYSCFG "/machine/soc"

typedef struct gpio_pin {
    uint32_t gpio_group;
    uint32_t gpio_group_addr;
    uint8_t pin_number;
} gpio_pin;

//static void* set_test_data(unsigned int gpio_group, unsigned int gpio_group_addr, unsigned int gpio_pin_num, gpio_pin* pin_obj) {
//    pin_obj->gpio_group = gpio_group;
//    pin_obj->gpio_group_addr = gpio_group_addr;
//    pin_obj->pin_number = gpio_pin_num;
//    return (void*)pin_obj;
//}

unsigned int reset_vals[GPIO_NUM_REGS] = {
    RESET_CRL,
    RESET_CRH,
    RESET_IDR,
    RESET_ODR,
    RESET_BSRR,
    RESET_BRR,
    RESET_LCKR
};

/*Helper function for writel: pass the GPIO_X group for offset (register address) computing*/
static void gpio_writel(unsigned int gpio_port, unsigned int reg, uint32_t val) {
    writel(gpio_port + reg, val);
}

/*Helper function for readl: pass the GPIO_X group for offset computing*/
static uint32_t gpio_readl(unsigned int gpio_port, unsigned int reg) {
    return readl(gpio_port + reg);
}

//static void gpio_set_irq(unsigned int gpio_group, int pin_number, int pin_level) {
//    //unsigned int pin_number_real = (gpio_group * GPIO_NUM_PINS * pin_number);
//    g_autofree char* pin_group_name = g_strdup_printf("/machine/soc/GPIO%c", gpio_group + 'A');
//    qtest_set_irq_in(global_qtest, pin_group_name, NULL, pin_number, pin_level);
//}

//static bool gpio_get_irq(unsigned int gpio_group, int pin_number) {
//    gpio_group = 0;
//    bool irq_stat = qtest_get_irq(global_qtest, (pin_number));
//    return irq_stat;
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
    // Write unuseful data on every register
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

//static void stm32f2xx_test_input_mode_gpio(const void* data) {
//    gpio_pin *test_pin = (gpio_pin*)data;
//    /*variables for RESET check*/
//    unsigned int gpio_addr;
//    unsigned int reg_addr;
//    unsigned int read_data;
//
//    /*Variables for register reading*/
//    unsigned int crx_pin_pos;
//    unsigned int crx_reg;
//    unsigned int idr_reg_bit;
//
//    /*GPIO utils*/
//    unsigned int gpio_group = test_pin->gpio_group;
//    unsigned int pin_number = test_pin->pin_number;
//    unsigned int gpio_group_addr = test_pin->gpio_group_addr;
//
//    /*Perform a system reset and check if all registers are in correct state*/
//    stm32f2xx_system_reset();
//    for(gpio_addr = GPIO_A_ADDR; gpio_addr <= GPIO_E_ADDR; gpio_addr += GPIO_PORT_SIZE) {
//        for(reg_addr = GPIOx_CRL; reg_addr <= GPIOx_LCKR; reg_addr += GPIO_REG_SIZE) {
//            read_data = gpio_readl(gpio_addr, reg_addr);
//            g_assert_cmphex(read_data, ==, reset_vals[(reg_addr/GPIO_REG_SIZE)]);
//        }
//    }
//
//    //g_autofree char* gpio_group_path = g_strdup_printf("/machine/soc/GPIO%c", gpio_group + 'A');
//    //qtest_irq_intercept_in(global_qtest, gpio_group_path);
//
//    
//    /*Get CRH or CRL register according to pin number*/
//    crx_reg = (pin_number >= (GPIO_NUM_PINS / 2)) ? GPIOx_CRH : GPIOx_CRL;
//    crx_pin_pos = (crx_reg == GPIOx_CRH) ? pin_number - (GPIO_NUM_PINS/2) : pin_number;
//    gpio_writel(gpio_group_addr, crx_reg, (GPIOx_CNF_INPUT | GPIOx_MODE_INPUT) << (crx_pin_pos * 4));
//
//    /*Drive pin line HIGH*/
//    gpio_set_irq(gpio_group, pin_number, HIGH);
//    //g_assert_true(gpio_get_irq(gpio_group, pin_number));
//
//    /*Check corresponding IDR bit is set to 1*/
//    idr_reg_bit = gpio_readl(gpio_group_addr, GPIOx_IDR) >> pin_number;
//    g_assert_cmphex(idr_reg_bit, ==, 1);
//
//    /*Drive pin line LOW*/
//    gpio_set_irq(gpio_group, pin_number, LOW);
//    //g_assert_false(gpio_get_irq(gpio_group, pin_number));
//
//    /*Check corresponding IDR bit is set to 0*/
//    idr_reg_bit = gpio_readl(gpio_group_addr, GPIOx_IDR) >> pin_number;
//    g_assert_cmphex(idr_reg_bit, ==, 0);
//}

/*static void stm32f2xx_test_gpio_output_mode(const void *data) {

}*/


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
    //gpio_pin pin_under_test;
    g_test_init(&argc, &argv, NULL);
    g_test_set_nonfatal_assertions();
    qtest_add_func("stm32f2xx/gpio/stm32f2xx_test_reset_values", stm32f2xx_test_reset_values);
    //qtest_add_data_func("stm32f2xx/gpio/stm32f2xx_test_input_mode_gpioa", set_test_data(GPIO_A_GROUP, GPIO_A_ADDR, GPIO_PIN_0, &pin_under_test), stm32f2xx_test_input_mode_gpio);
    //qtest_add_data_func("stm32f2xx/gpio/stm32f2xx_test_input_mode_gpiob", set_test_data(GPIO_B_GROUP, GPIO_B_ADDR, GPIO_PIN_5, &pin_under_test), stm32f2xx_test_input_mode_gpio);
    //qtest_add_data_func("stm32f2xx/gpio/stm32f2xx_test_input_mode_gpioc", set_test_data(GPIO_C_GROUP, GPIO_C_ADDR, GPIO_PIN_10, &pin_under_test), stm32f2xx_test_input_mode_gpio);
    //qtest_add_data_func("stm32f2xx/gpio/stm32f2xx_test_input_mode_gpiod", set_test_data(GPIO_D_GROUP, GPIO_D_ADDR, GPIO_PIN_15, &pin_under_test), stm32f2xx_test_input_mode_gpio);
    qtest_start("-machine stm32vldiscovery");
    ret = g_test_run();
    qtest_end();

    return ret;
}
