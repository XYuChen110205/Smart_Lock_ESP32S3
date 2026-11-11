#include "vibration.h"
#include "pins.h"
#include "buzzer.h"
#include "cloud.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "VIB"

#define VIB_SAMPLE_MS        10    // 每 10ms 采样一次
#define VIB_DEBOUNCE_MS      30    // 消抖
#define VIB_BURST_GAP_MS     500   // 两次震动间隔超过它算新的一串
#define VIB_WINDOW_MS        5000  // 统计窗口
#define VIB_MEDIUM_BURSTS    2     // 窗口内 2 串 -> 中
#define VIB_SEVERE_BURSTS    4     // 窗口内 4 串 -> 重
#define VIB_SEVERE_ACTIVE_MS 1500  // 持续震动 1.5s -> 重
#define VIB_COOLDOWN_MS      5000  // 报警冷却，避免一直响

static void vib_task(void *arg)
{
    (void)arg;

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << VIBRATION_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);

    int idle = gpio_get_level(VIBRATION_GPIO);
    int last = idle;
    TickType_t last_edge = 0;
    TickType_t active_start = 0;
    TickType_t last_alert = 0;
    TickType_t window_start = xTaskGetTickCount();
    int bursts = 0;

    ESP_LOGI(TAG, "ready on GPIO%d (idle=%d)", VIBRATION_GPIO, idle);

    while (1) {
        TickType_t now = xTaskGetTickCount();
        int level = gpio_get_level(VIBRATION_GPIO);

        // 检测边沿，按“串”计数
        if (level != last) {
            if (last_edge == 0 ||
                (now - last_edge) >= pdMS_TO_TICKS(VIB_DEBOUNCE_MS)) {
                if (last_edge == 0 ||
                    (now - last_edge) > pdMS_TO_TICKS(VIB_BURST_GAP_MS)) {
                    bursts++;
                }
                last_edge = now;
            }
            last = level;
        }

        // 记录“偏离静止电平”的持续时间
        if (level != idle) {
            if (active_start == 0) {
                active_start = now;
            }
        } else {
            active_start = 0;
        }

        // 统计窗口滚动
        if ((now - window_start) > pdMS_TO_TICKS(VIB_WINDOW_MS)) {
            bursts = 0;
            window_start = now;
        }

        TickType_t active_ms = 0;
        if (active_start != 0) {
            active_ms = (now - active_start) * portTICK_PERIOD_MS;
        }

        int cls = 0;
        if (active_ms >= VIB_SEVERE_ACTIVE_MS || bursts >= VIB_SEVERE_BURSTS) {
            cls = 3;
        } else if (bursts >= VIB_MEDIUM_BURSTS) {
            cls = 2;
        } else if (bursts >= 1) {
            cls = 1;
        }

        // 中/重报警（带冷却）
        if (cls >= 2 && (now - last_alert) > pdMS_TO_TICKS(VIB_COOLDOWN_MS)) {
            last_alert = now;
            ESP_LOGW(TAG, "ALERT class=%d bursts=%d active=%lums",
                     cls, bursts, (unsigned long)active_ms);
            Cloud_Publish(cls == 2 ? "{\"event\":\"vibration\",\"level\":2}"
                                   : "{\"event\":\"vibration\",\"level\":3}");

            if (cls == 2) {
                Beep_Pwm(2200, 100);
                vTaskDelay(pdMS_TO_TICKS(80));
                Beep_Pwm(2200, 100);
            } else {
                for (int i = 0; i < 3; i++) {
                    Beep_Pwm(3000, 150);
                    vTaskDelay(pdMS_TO_TICKS(80));
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(VIB_SAMPLE_MS));
    }
}

void Vibration_Start(void)
{
    xTaskCreatePinnedToCore(vib_task, "vib", 3072, NULL, 7, NULL, 1);
}
