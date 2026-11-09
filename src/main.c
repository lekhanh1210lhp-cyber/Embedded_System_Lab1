
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define Y0_NODE DT_ALIAS(led1)

#if !DT_NODE_EXISTS(Y0_NODE)
#error "led1 alias is not defined"
#endif

static const struct gpio_dt_spec y0 =
    GPIO_DT_SPEC_GET(Y0_NODE, gpios);

int main(void)
{
    int ret;

    if (!gpio_is_ready_dt(&y0)) {
        printk("ERROR: Y0 GPIO is not ready\n");
        return 0;
    }

    ret = gpio_pin_configure_dt(&y0, GPIO_OUTPUT_INACTIVE);

    if (ret < 0) {
        printk("ERROR: Y0 configuration failed: %d\n", ret);
        return 0;
    }

    printk("Y0 GPIO test started\n");

    while (1) {
        printk("Y0 ON\n");
        ret = gpio_pin_set_dt(&y0, 1);

        if (ret < 0) {
            printk("Y0 ON failed: %d\n", ret);
        }

        k_msleep(1000);

        printk("Y0 OFF\n");
        ret = gpio_pin_set_dt(&y0, 0);

        if (ret < 0) {
            printk("Y0 OFF failed: %d\n", ret);
        }

        k_msleep(1000);
    }

    return 0;
}
