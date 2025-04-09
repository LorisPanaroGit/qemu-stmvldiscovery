/*
meson test qtest-arm/stm32f2xx_gpio-test -> run single meson test for GPIO
*/

#include "qemu/osdep.h"
#include "libqtest-single.h"

#define GPIO_NUM_PORTS 5
#define GPIO_NUM_REGS  7

#define GPIO_PORT_SIZE 0x400
#define GPIO_REG_SIZE  0x4
#define GPIO_BASE_ADDR 0x40010800

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
    uint32_t gpio_line;
    uint8_t pin_number;
} gpio_pin;

unsigned int reset_vals[GPIO_NUM_REGS] = {
    RESET_CRL,
    RESET_CRH,
    RESET_IDR,
    RESET_ODR,
    RESET_BSRR,
    RESET_BRR,
    RESET_LCKR
};

static unsigned int get_gpio_id(uint32_t gpio_addr) {
    return (gpio_addr - GPIO_BASE_ADDR) / GPIO_PORT_SIZE;
}

/*test_data() just builds a number by taking the offset of the GPIOx_line and concatenates the pin number*/
static void* test_data(uint32_t gpio_line, uint8_t pin_num, gpio_pin* pin_obj) {
    pin_obj->gpio_line = gpio_line;
    pin_obj->pin_number = pin_num;
    return (void*)(pin_obj);
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
static void gpio_set_irq(unsigned int gpio, int num, int level) {
    g_autofree char *name = g_strdup_printf("/machine/soc/GPIO%c",
                                            get_gpio_id(gpio) + 'A');
    qtest_set_irq_in(global_qtest, name, NULL, num, level);
}

/*static void disconnect_all_pins(unsigned int gpio)
{
    g_autofree char *path = g_strdup_printf("/machine/soc/gpio%c",
                                            get_gpio_id(gpio) + 'a');
    QDict *r;

    r = qtest_qmp(global_qtest, "{ 'execute': 'qom-set', 'arguments': "
        "{ 'path': %s, 'property': 'disconnected-pins', 'value': %d } }",
        path, 0xFFFF);
    g_assert_false(qdict_haskey(r, "error"));
    qobject_unref(r);
}*/

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
    gpio_pin *test_pin = (gpio_pin*)data;
    uint32_t pin_num = test_pin->pin_number;
    uint32_t gpio_addr = test_pin->gpio_line;
    uint32_t gpio_id = get_gpio_id(test_pin->gpio_line);
    uint32_t gpio_crx_reg = (pin_num >= 8) ? GPIOx_CRH : GPIOx_CRL;
    uint32_t idr_pin_val;

    qtest_irq_intercept_in(global_qtest, SYSCFG);
    
    /*Set GPIOx[GPIO_PIN_<number>] as INPUT pin -> ideally, the shift works for pin_num < 8 but the offset must be subtracted if pin_num falls into CRH*/
    gpio_writel(gpio_addr, gpio_crx_reg, (GPIOx_MODE_INPUT | GPIOx_CNF_INPUT) << (pin_num * 4));

    /*Set digital line to 1; check IDR is set to 1*/
    gpio_set_irq(gpio_addr, pin_num, 1);
    idr_pin_val = gpio_readl(gpio_addr, GPIOx_IDR) >> pin_num;
    printf("GPIO ID: %u\n", gpio_id);
    printf("Pin: %u\n", pin_num);
    printf("Level of the line: %u\n", get_irq((gpio_id * GPIO_NUM_PINS) + pin_num));
    g_assert_cmphex(idr_pin_val, ==, 1);
    g_assert_true(get_irq((gpio_id * GPIO_NUM_PINS) + pin_num));
    
    /*Set digital line to 0; check IDR is set to 0*/
    gpio_set_irq(gpio_addr, pin_num, 0);
    g_assert_false(get_irq((gpio_id * GPIO_NUM_PINS) + pin_num));
    idr_pin_val = gpio_readl(gpio_addr, GPIOx_IDR) >> pin_num;
    g_assert_cmphex(idr_pin_val, ==, 0);

    /*Clean the test*/
    //disconnect_all_pins(gpio_addr);
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
    gpio_pin pin_under_test;
    g_test_init(&argc, &argv, NULL);
    g_test_set_nonfatal_assertions();
    qtest_add_func("stm32f2xx/gpio/stm32f2xx_test_reset_values", stm32f2xx_test_reset_values);
    qtest_add_data_func("stm32f2xx/gpio/stm32f2xx_test_input_mode_gpioa", test_data(GPIO_A_ADDR, GPIO_PIN_1, &pin_under_test), stm32f2xx_test_gpio_input_mode);
    qtest_start("-machine stm32vldiscovery");
    ret = g_test_run();
    qtest_end();

    return ret;
}
