


#include "servo.h"

#include "pins.h"
#include "iot_servo.h"     // 官方 espressif/servo 组件


static servo_handle_t s_servo;//servo句柄





void Servo_Init(void)
{
    servo_config_t cfg = SERVO_CONFIG_DEFAULT(
        LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, LEDC_CHANNEL_0, SERVO_GPIO);
    iot_servo_new(&cfg, &s_servo);
    Servo_Close();
}




void Servo_Open(void)    // 开锁角度
{
    iot_servo_write_angle(s_servo, 90);

}


void Servo_Close(void)   // 关锁角度
{
iot_servo_write_angle(s_servo, 0); 
}






