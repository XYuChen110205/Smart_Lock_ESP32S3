

#ifndef __SERVO_H__
#define __SERVO_H__

#include "esp_err.h"





void Servo_Init(void);
void Servo_Open(void);    // 开锁角度
void Servo_Close(void);   // 关锁角度

#endif
