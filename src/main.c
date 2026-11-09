
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/atomic.h>

/* Get GPIO information from Devicetree aliases */
#define LED3_NODE DT_ALIAS(led0)
#define Y0_NODE   DT_ALIAS(led1)

#if !DT_NODE_EXISTS(LED3_NODE) || !DT_NODE_EXISTS(Y0_NODE)
#error "Devicetree aliases led0 or led1 are not defined"
#endif

/* GPIO specifications */
static const struct gpio_dt_spec led3 =
    GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static const struct gpio_dt_spec y0 =
    GPIO_DT_SPEC_GET(Y0_NODE, gpios);

/* Shared variable for synchronization between threads */
static atomic_t led_state = ATOMIC_INIT(0);


/*
 * Thread 1: Control LED3
 * ON for 2 seconds, OFF for 2 seconds.
 */
void thread_led_blinky(void *p1, void *p2, void *p3)
{
    int ret;

    if (!gpio_is_ready_dt(&led3)) {
        printk("ERROR: LED3 GPIO device is not ready\n");
        return;
    }

    ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR: Cannot configure LED3 (%d)\n", ret);
        return;
    }

    while (1) {
        /* Turn LED3 ON */
        gpio_pin_set_dt(&led3, 1);
        atomic_set(&led_state, 1);

        k_msleep(2000);

        /* Turn LED3 OFF */
        gpio_pin_set_dt(&led3, 0);
        atomic_set(&led_state, 0);

        k_msleep(2000);
    }
}


/*
 * Thread 2: Control Output Y0
 * Y0 follows the state of LED3.
 * Poll the shared variable every 10 ms.
 */
void thread_y0_follow(void *p1, void *p2, void *p3)
{
    int ret;
    int current_state;
    int previous_state = -1;

    if (!gpio_is_ready_dt(&y0)) {
        printk("ERROR: Y0 GPIO device is not ready\n");
        return;
    }

    ret = gpio_pin_configure_dt(&y0, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR: Cannot configure Y0 (%d)\n", ret);
        return;
    }

    while (1) {
        /* Read LED3 state from the shared atomic variable */
        current_state = atomic_get(&led_state);

        /* Update Y0 only when the state changes */
        if (current_state != previous_state) {
            gpio_pin_set_dt(&y0, current_state);

            previous_state = current_state;
        }

        /* Check the shared variable every 10 ms */
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
