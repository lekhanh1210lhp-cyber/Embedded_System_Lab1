#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_ALIAS(led0)

#if !DT_NODE_EXISTS(LED_NODE)
#error "No led0 alias defined in devicetree"
#endif

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

void thread_led_blinky(void *p1, void *p2, void *p3)
{
    int ret;

    if (!gpio_is_ready_dt(&led)) {
        printk("LED GPIO controller is not ready\n");
        return;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Failed to configure LED: %d\n", ret);
        return;
    }

    printk("Exercise 1: Asymmetric LED Blink on PE4\n");

    while (1) {
        gpio_pin_set_dt(&led, 1);
        k_msleep(1000);

        gpio_pin_set_dt(&led, 0);
        k_msleep(2000);
    }
}

K_THREAD_DEFINE(blinky_thread_id, 1024, thread_led_blinky, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    return 0;
}