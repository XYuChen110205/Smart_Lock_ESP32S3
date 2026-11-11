


#include "lock_action.h"

#include "ui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "servo.h"

 #include "buzzer.h"
#include "syn6288.h"
#include "cloud.h"
#include "tts_text.h"


//开锁成功
void Lock_Open(void)
{
    printf("LOCK: open\n");
    UI_Unlock();//解锁页面
    Servo_Open();//跟着舵机转动
    Beep_Pwm(2500,120);//跟着蜂鸣器响应
    Syn6288_PlayText((const char *)TTS_UNLOCK);//语音播报开锁成功
    Cloud_Publish("{\"event\":\"unlock\"}");//云上报
    vTaskDelay(pdMS_TO_TICKS(3000));   // 保持 3 秒
    Servo_Close();//之后自动回锁
    printf("LOCK: close\n");
}

//开锁失败
void Lock_WrongPassword(void)
{
    UI_Message("WRONG", "PASSWORD");
    Syn6288_PlayText((const char *)TTS_WRONG);
    vTaskDelay(pdMS_TO_TICKS(1500));
}










