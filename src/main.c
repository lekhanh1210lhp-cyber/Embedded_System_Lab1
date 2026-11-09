#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

/* Định nghĩa alias từ devicetree */
#define LED_NODE DT_ALIAS(led0)
#define Y0_NODE DT_ALIAS(led1)

#if !DT_NODE_EXISTS(LED_NODE) || !DT_NODE_EXISTS(Y0_NODE)
#error "Devicetree aliases (led0, led1) are not fully defined"
#endif

/* Khai báo thông tin GPIO cho LED3 và Output Y0 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(Y0_NODE, gpios);

/* Biến dùng chung để đồng bộ 2 luồng. Từ khóa volatile là bắt buộc. */
volatile bool shared_led_state = false;

/* Luồng 1: Điều khiển LED3 bật 2s, tắt 2s và cập nhật biến dùng chung */
void thread_led_blinky(void *p1, void *p2, void *p3)
{
    if (!gpio_is_ready_dt(&led0)) {
        return;
    }

    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    while (1) {
        /* Bật LED3 và báo hiệu biến dùng chung */
        shared_led_state = true;
        gpio_pin_set_dt(&led0, 1);
        k_msleep(2000);

        /* Tắt LED3 và báo hiệu biến dùng chung */
        shared_led_state = false;
        gpio_pin_set_dt(&led0, 0);
        k_msleep(2000);
    }
}

/* Luồng 2: Liên tục đọc biến dùng chung để mô phỏng trạng thái cho Output Y0 */
void thread_y0_follow(void *p1, void *p2, void *p3)
{
    if (!gpio_is_ready_dt(&led1)) {
        return;
    }

    gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);

    while (1) {
        /* Đọc trạng thái từ biến và cập nhật lên phần cứng Output Y0 */
        if (shared_led_state) {
            gpio_pin_set_dt(&led1, 1);
        } else {
            gpio_pin_set_dt(&led1, 0);
        }
        
        /* Nghỉ 10ms rồi mới kiểm tra lại */
        k_msleep(10);
    }
}

/* Đăng ký cả 2 luồng chạy độc lập nhưng giao tiếp qua shared_led_state */
K_THREAD_DEFINE(blinky_thread_id, 1024, thread_led_blinky, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(y0_follow_thread_id, 1024, thread_y0_follow, NULL, NULL, NULL, 7, 0, 0);

int main(void)
{
    return 0;
}