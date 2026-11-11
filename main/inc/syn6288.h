#ifndef __SYN6288_H__
#define __SYN6288_H__

#include "esp_err.h"
#include "pins.h"

// 用 pins.h 里的 TTS 引脚
#define SYN_UART_PORT TTS_UART_PORT
#define SYN_TX_PIN    TTS_TX
#define SYN_RX_PIN    TTS_RX

esp_err_t Syn6288_Init(void);
void Syn6288_PlayText(const char *text);

#endif
