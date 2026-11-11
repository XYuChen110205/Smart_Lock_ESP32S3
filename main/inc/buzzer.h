#ifndef __BUZZER_H__
#define __BUZZER_H__

#include <stdint.h>

// 无源蜂鸣器：信号脚来自 pins.h 的 BUZZER_GPIO，用 LEDC 产生方波
void Buzzer_Init(void);
void Beep_Pwm(uint32_t freq_hz, uint32_t duration_ms);
void Buzzer_Off(void);

#endif
