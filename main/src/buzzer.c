#include "buzzer.h"
#include "pins.h"

#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_check.h"

#define BUZZER_MODE       LEDC_LOW_SPEED_MODE
//官方规定s3只能填写低速组


#define BUZZER_TIMER      LEDC_TIMER_1
#define BUZZER_CHANNEL    LEDC_CHANNEL_1

#define BUZZER_DEFAULT_FREQ 2700
#define BUZZER_RES          LEDC_TIMER_10_BIT //10位，占空比0-1023
#define BUZZER_DUTY_MAX     ((1 << 10) - 1)
#define BUZZER_DUTY         2     // 音量 0~1023，越小越轻；太吵改 5

static bool s_initialized;

void Buzzer_Init(void)
{
    if (s_initialized) {
        return;
    }

    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_MODE,
        .duty_resolution = BUZZER_RES,
        .timer_num = BUZZER_TIMER,
        .freq_hz = BUZZER_DEFAULT_FREQ,//初始频率
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = BUZZER_MODE,
        .channel = BUZZER_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = BUZZER_TIMER,
        .duty = 0,//设置一开始不响
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));

    s_initialized = true;
}

void Beep_Pwm(uint32_t freq_hz, uint32_t duration_ms)
{
    if (!s_initialized) {
        Buzzer_Init();
    }
    if (freq_hz < 100) {
        freq_hz = 100;//频率设置
    }

    //设置占空比，50%
    ledc_set_freq(BUZZER_MODE, BUZZER_TIMER, freq_hz);
    //提交，duty生效
    ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, BUZZER_DUTY);
    ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);
    // 合并版本（备注）：ledc_set_duty_and_update(BUZZER_MODE, BUZZER_CHANNEL, BUZZER_DUTY_MAX / 2, 0);


    //响多少ms
    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    //关闭，设置为0%并提交，蜂鸣器停止
    Buzzer_Off();
}

void Buzzer_Off(void)
{
    //关闭，占空比设0+update
    ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, 0);
    ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL);
}


//定时器通道LEDC_TIMER_0/LEDC_CHANNEL_)
//所以这里直接用TIMER_1/CHANNEL_1
