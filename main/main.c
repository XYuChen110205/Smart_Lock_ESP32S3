#include <stdio.h>
#include "o1st7789.h"
#include "lvgl_app.h"
#include "ui.h"

#include "app_fsm.h"
#include "app_event.h"
#include "freertos/task.h"
#include "mock_input.h"
#include "servo.h"
#include "buzzer.h"
#include "fp_app.h"
#include "keypad.h"
#include "vibration.h"
#include "syn6288.h"
#include "rfid_app.h"
#include "cloud.h"
#include "tts_text.h"



void app_main(void)
{
    lcd_init();
    LCD_Fill(0xFFFF);                 // 白屏测试：确认背光/屏幕
    vTaskDelay(pdMS_TO_TICKS(800));
    Servo_Init();
    Buzzer_Init();
    Syn6288_Init();
    // lcd_test();

    lvgl_ui_init();
    // UI_Menu(0);


    FSM_Init();

    // 先建队列、起唯一的大脑，再启动输入源
    AppEvent_Init();
    FSM_Task_Start();
    // Cloud_Start();   // 先关云：WiFi 启动会 brownout，换稳压 5V/2A 再开

    Fingerprint_Start();
    Keypad_Start();
    MockInput_Start();
    // Vibration_Start();   // 测键盘时先关掉，避免误触发一直响
    Rfid_Start();

    // 开机测试：语音播报（中文）
    vTaskDelay(pdMS_TO_TICKS(1500));
    Syn6288_PlayText((const char *)TTS_TEST);
}




