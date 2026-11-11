#include "fp_app.h"
#include "fpm383c.h"
#include "app_fsm.h"
#include "app_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define TAG "FP_APP"

static void fp_task(void *arg)
{
    (void)arg;

    // 1) 初始化 UART1
    esp_err_t err = fpm383c_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "init failed: %s", esp_err_to_name(err));
        vTaskDelete(NULL);
        return;
    }

    // 2) 等模块在线，心跳失败就重试
    while (fpm383c_heartbeat() != ESP_OK) {
        ESP_LOGW(TAG, "no heartbeat, retry...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_LOGI(TAG, "fingerprint ready");

    // 3) 检测手指 -> 匹配 -> 上报 FSM
    bool last = false;
    while (1) {
        bool present = false;

        if (fpm383c_finger_present(&present) == ESP_OK) {
            if (present && !last) {
                fpm383c_match_result_t r;
                if (fpm383c_match_sync(&r, 10000) == ESP_OK) {
                    ESP_LOGI(TAG, "match id=%u score=%u", r.match_id, r.match_score);
                    AppEvent_Post(EVT_FINGER_OK);
                } else {
                    AppEvent_Post(EVT_FINGER_FAIL);
                }
            }
            last = present;
        } else {
            last = false;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void Fingerprint_Start(void)
{
    xTaskCreatePinnedToCore(fp_task, "fp", 4096, NULL, 5, NULL, 1);
}
