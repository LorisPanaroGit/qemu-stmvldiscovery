#include "qemu/osdep.h"
#include "libqtest-single.h"

static void stm32f2xx_system_reset() {

}

static void stm32f2xx_test_gpio_output_mode(const void *data) {

}

static void stm32f2xx_test_gpio_input_mode(const void *data) {

}

static void stm32f2xx_test_pull_up_pull_down(const void *data) {

}

static void stm32f2xx_test_push_pull(const void *data) {

}

static void stm32f2xx_test_open_drain(const void *data) {

}

static void stm32f2xx_test_bsrr_brr(const void *data) {

}

int main(int argc, char **argv) {
    int ret;
    g_test_init(&argc, &argv, NULL);
    g_test_set_nonfatal_assertions();
    ret = g_test_run();
    qtest_end();

    return ret;
}
