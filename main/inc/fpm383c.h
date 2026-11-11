


#ifndef __FPM383C_H__
#define __FPM383C_H__


#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/uart.h"

#define FPM383C_UART_PORT UART_NUM_1
#define FPM383C_BAUD      57600

typedef struct {
    uint16_t match_result;   // 1 = 匹配成功
    uint16_t match_score;
    uint16_t match_id;
} fpm383c_match_result_t;

esp_err_t fpm383c_init(void);
void fpm383c_test(void);      // 发 0x0135 并打印帧
esp_err_t fpm383c_heartbeat(void);
void fpm383c_probe(void);     // 自动扫波特率 + TX/RX 两种接法


esp_err_t fpm383c_finger_present(bool *present);



esp_err_t fpm383c_match_sync(fpm383c_match_result_t *result, uint32_t timeout_ms);





#endif
