#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_ALIAS(led0)
#define Y0_NODE DT_ALIAS(led1)

#if !DT_NODE_EXISTS(LED_NODE) || !DT_NODE_EXISTS(Y0_NODE)
#error "Devicetree aliases (led0, led1) are not fully defined"
#endif

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(Y0_NODE, gpios);

volatile bool shared_led_state = false;

void thread_led_blinky(void *p1, void *p2, void *p3)
{
    if (!gpio_is_ready_dt(&led0)) {
        return;
    }

    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    while (1) {
        shared_led_state = true;
        gpio_pin_set_dt(&led0, 1);
        k_msleep(2000);

        shared_led_state = false;
        gpio_pin_set_dt(&led0, 0);
        k_msleep(2000);
    }
}

void thread_y0_follow(void *p1, void *p2, void *p3)
{
    if (!gpio_is_ready_dt(&led1)) {
        return;
    }

    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);

    while (1) {
        if (shared_led_state) {
            gpio_pin_set_dt(&led1, 1);
        } else {
            gpio_pin_set_dt(&led1, 0);
        }
        
        k_msleep(10);
    }
}

K_THREAD_DEFINE(blinky_thread_id, 1024, thread_led_blinky, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(y0_follow_thread_id, 1024, thread_y0_follow, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    return 0;
}