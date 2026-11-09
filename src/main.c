
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/atomic.h>

/* Devicetree aliases */
#define LED_NODE DT_ALIAS(led0)
#define Y0_NODE  DT_ALIAS(led1)

#if !DT_NODE_EXISTS(LED_NODE) || !DT_NODE_EXISTS(Y0_NODE)
#error "Devicetree aliases led0 and led1 are not fully defined"
#endif

/* GPIO specifications */
static const struct gpio_dt_spec led0 =
    GPIO_DT_SPEC_GET(LED_NODE, gpios);

static const struct gpio_dt_spec led1 =
    GPIO_DT_SPEC_GET(Y0_NODE, gpios);

/* Shared variable: initial state ACTIVE */
static atomic_t shared_led_state = ATOMIC_INIT(1);


/* Thread 1: LED3 ON for 2 seconds, OFF for 2 seconds */
void thread_led_blinky(void *p1, void *p2, void *p3)
{
    int ret;

    if (!gpio_is_ready_dt(&led0)) {
        printk("ERROR: LED3 GPIO is not ready\n");
        return;
    }

    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);

    if (ret < 0) {
        printk("ERROR: Cannot configure LED3: %d\n", ret);
        return;
    }

    while (1) {
        /* LED3 ON */
        gpio_pin_set_dt(&led0, 1);
        atomic_set(&shared_led_state, 1);

        k_msleep(2000);

        /* LED3 OFF */
        gpio_pin_set_dt(&led0, 0);
        atomic_set(&shared_led_state, 0);

        k_msleep(2000);
    }
}


/* Thread 2: Y0 follows LED3 state */
void thread_y0_follow(void *p1, void *p2, void *p3)
{
    int ret;
    int current_state;
    int previous_state = -1;

    if (!gpio_is_ready_dt(&led1)) {
        printk("ERROR: Y0 GPIO is not ready\n");
        return;
    }

    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);

    if (ret < 0) {
        printk("ERROR: Cannot configure Y0: %d\n", ret);
        return;
    }

    while (1) {
        /* Read shared state */
        current_state = atomic_get(&shared_led_state);

        /* Update Y0 when LED3 state changes */
        if (current_state != previous_state) {
            gpio_pin_set_dt(&led1, current_state);
            previous_state = current_state;
        }

        /* Poll every 10 ms */
        k_msleep(10);
    }
}


/* Create two independent threads */
K_THREAD_DEFINE(
    blinky_thread_id,
    1024,
    thread_led_blinky,
    NULL, NULL, NULL,
    7, 0, 0
);

K_THREAD_DEFINE(
    y0_follow_thread_id,
    1024,
    thread_y0_follow,
    NULL, NULL, NULL,
    7, 0, 0
);


int main(void)
{
    return 0;
}
