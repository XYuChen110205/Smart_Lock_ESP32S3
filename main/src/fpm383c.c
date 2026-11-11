


#include "fpm383c.h"

#include "pins.h"
#include "driver/uart.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>

/*
帧头固定
F1 1F E2 2E B6 6B A8 8A
应用层长度=应用层数据的字节数（不包含帧头/长度/帧头校验）
帧头校验= （帧头+长度各字节之和+校验）的低8位= 0，校验= 0x100-（sum&0xFF）
应用层数据（请求） = 校验密码（4） + 命令（2） +数据（N） + 校验和（1） ；
N=0时候，长度=7
应用层校验和，同样为0x100- 前面所有字节之和&0xFF

01 11 01 和= 

*/




//校验和函数
static uint8_t fpm_checksum(const uint8_t *buf, size_t len)
{

    //数据相加
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return (uint8_t)(0x100 - sum);//这里不懂
}

//拼帧框架
//第一，固定帧头
static const uint8_t s_header[8] = {0xF1,0x1F,0xE2,0x2E,0xB6,0x6B,0xA8,0x8A};

// 返回整帧长度
static int fpm_build(uint8_t cmd1, uint8_t cmd2,
                     const uint8_t *data, uint16_t data_len,
                     uint8_t *out, size_t out_size)

{
    uint16_t app_len = 4 + 2 + data_len + 1;      // 密码+命令+数据+校验
    size_t total = 8 + 2 + 1 + app_len;
    if (out_size < total) return -1;

    size_t i = 0;

    // 1) 帧头 8 字节
    memcpy(out + i, s_header, 8); i += 8;

    // 2) 应用层长度（大端：高字节在前）
    out[i++] = (uint8_t)(app_len >> 8);
    out[i++] = (uint8_t)(app_len & 0xFF);

    // 3) 帧头校验（覆盖 帧头+长度 共 10 字节）
    out[i] = fpm_checksum(out, i);
    i++;

    // 4) 密码 4 字节（默认 0）
    out[i++] = 0; out[i++] = 0; out[i++] = 0; out[i++] = 0;

    // 5) 命令 2 字节
    out[i++] = cmd1;
    out[i++] = cmd2;

    // 6) 数据
    if (data_len > 0) {
        memcpy(out + i, data, data_len);
        i += data_len;
    }

    // 7) 应用层校验（覆盖 密码+命令+数据）
    out[i] = fpm_checksum(out + 11, i - 11);   // 11 = 8+2+1
    i++;

    return (int)i;
}

//发送
static esp_err_t fpm_send(uint8_t cmd1, uint8_t cmd2,
                          const uint8_t *data, uint16_t data_len)
{
    uint8_t frame[64];
    int n = fpm_build(cmd1, cmd2, data, data_len, frame, sizeof(frame));//这个的功能是什么
    if (n < 0) return ESP_ERR_INVALID_SIZE;

    uart_write_bytes(FPM383C_UART_PORT, frame, n);
    for(int k =0; k< n; k++)
    {
        printf("%02X ",frame[k]);
    }
    printf("\n");
    return ESP_OK;
}


//初始化
esp_err_t fpm383c_init(void)
{
    uart_config_t cfg = {
        .baud_rate = FPM383C_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    //官方模板
    ESP_RETURN_ON_ERROR(uart_param_config(FPM383C_UART_PORT, &cfg), "FPM", "param");
    ESP_RETURN_ON_ERROR(uart_set_pin(FPM383C_UART_PORT, FP_TX, FP_RX,
                        UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE), "FPM", "pin");
    gpio_set_pull_mode(FP_RX, GPIO_PULLUP_ONLY);
    ESP_RETURN_ON_ERROR(uart_driver_install(FPM383C_UART_PORT, 1024, 1024, 0, NULL, 0),
                        "FPM", "install");

    uart_flush_input(FPM383C_UART_PORT);
    return ESP_OK;

}
static esp_err_t fpm_recv(
    uint8_t *cmd1,          // 出口1：把模块回复的命令1存到这里
    uint8_t *cmd2,          // 出口2：把模块回复的命令2存到这里
    uint32_t *err_code,     // 出口3：把错误码存到这里
    uint8_t *data,          // 出口4：把真正的有效数据存在这个数组里
    int data_max,           // 入口1：告诉我你的 data 数组有多大，防止我塞爆
    int *data_len,          // 出口5：告诉你有效数据实际上有几个字节
    uint32_t timeout_ms     // 入口2：最多等多少毫秒
)
{
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);

    // 1) 逐字节同步 8 字节帧头（测验帧头）
    int idx = 0;
    while (idx < 8) {
        uint8_t b;
        if (uart_read_bytes(FPM383C_UART_PORT, &b, 1, pdMS_TO_TICKS(50)) != 1) {
            if ((int32_t)(deadline - xTaskGetTickCount()) <= 0) {
                return ESP_ERR_TIMEOUT;
            }
            continue;
        }
        if (b == s_header[idx]) {
            idx++;
        } else {
            idx = (b == s_header[0]) ? 1 : 0;
        }
    }

    // 2) 长度2 + 头校验1
    uint8_t hdr[3];
    if (uart_read_bytes(FPM383C_UART_PORT, hdr, 3, pdMS_TO_TICKS(200)) != 3) {
        return ESP_ERR_TIMEOUT;   // 没有收到数据
    }
    uint16_t app_len = ((uint16_t)hdr[0] << 8) | hdr[1];   // 查看总长度
    if (app_len < 11 || app_len > 64) {
        return ESP_ERR_INVALID_SIZE;
    }

    // 3) 应用层数据
    uint8_t app[64];
    if (uart_read_bytes(FPM383C_UART_PORT, app, app_len, pdMS_TO_TICKS(300)) != app_len) {
        return ESP_ERR_TIMEOUT;
    }

    // 4) 校验
    uint8_t sum = 0;
    for (int i = 0; i < 8; i++) sum += s_header[i];
    sum += hdr[0]; sum += hdr[1];
    if ((uint8_t)(sum + hdr[2]) != 0) return ESP_ERR_INVALID_CRC;

    sum = 0;
    for (int i = 0; i < app_len - 1; i++) sum += app[i];
    if ((uint8_t)(sum + app[app_len - 1]) != 0) return ESP_ERR_INVALID_CRC;

    // 5) 解析
    // 2.提取命令，在15-16
    if (cmd1) *cmd1 = app[4];
    if (cmd2) *cmd2 = app[5];
    // 3.提取错误码，在17-20
    if (err_code) {
        *err_code = ((uint32_t)app[6] << 24) | ((uint32_t)app[7] << 16) |
                    ((uint32_t)app[8] << 8) | app[9];
    }
    // 4.计算有效数据长度：减去固定的11字节，剩下是数据
    int n = app_len - 11;
    if (n < 0) n = 0;
    if (n > data_max) n = data_max;   // 防止溢出
    if (data && data_len) {
        memcpy(data, &app[10], n);   // 把数据复制到data中
        *data_len = n;
    }
    return ESP_OK;
}


esp_err_t fpm383c_heartbeat(void)
{
    uint8_t c1, c2, data[16];
    uint32_t err;
    int n = 0;

    fpm_send(0x02, 0x03, NULL, 0);           // 获取模板数量
    esp_err_t e = fpm_recv(&c1, &c2, &err, data, sizeof(data), &n, 1000);
    if (e != ESP_OK) {
        printf("heartbeat: recv fail %s\n", esp_err_to_name(e));
        return e;
    }
    printf("heartbeat: cmd=%02X%02X err=%lu n=%d\n",
           c1, c2, (unsigned long)err, n);
    return (err == 0) ? ESP_OK : ESP_FAIL;
}


void fpm383c_probe(void)
{
    static const uint32_t bauds[] = { 57600, 115200, 9600, 19200, 38400, 230400 };
    static const int pins[2][2] = { { 16, 15 }, { 15, 16 } };  // {tx, rx}

    for (int b = 0; b < 6; b++) {
        for (int p = 0; p < 2; p++) {
            uart_set_pin(FPM383C_UART_PORT, pins[p][0], pins[p][1],
                         UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            gpio_set_pull_mode(pins[p][1], GPIO_PULLUP_ONLY);
            uart_set_baudrate(FPM383C_UART_PORT, bauds[b]);
            uart_flush_input(FPM383C_UART_PORT);
            vTaskDelay(pdMS_TO_TICKS(30));

            fpm_send(0x02, 0x03, NULL, 0);   // 心跳

            uint8_t buf[128];
            int total = 0;
            TickType_t t0 = xTaskGetTickCount();
            while ((xTaskGetTickCount() - t0) < pdMS_TO_TICKS(800)) {
                int n = uart_read_bytes(FPM383C_UART_PORT, buf + total,
                                        sizeof(buf) - total, pdMS_TO_TICKS(100));
                if (n > 0) {
                    total += n;
                }
                if (total >= (int)sizeof(buf)) {
                    break;
                }
            }

            printf("PROBE baud=%lu tx=%d rx=%d -> %d:",
                   (unsigned long)bauds[b], pins[p][0], pins[p][1], total);
            for (int i = 0; i < total; i++) {
                printf(" %02X", buf[i]);
            }
            printf("\n");
        }
    }

    uart_set_pin(FPM383C_UART_PORT, FP_TX, FP_RX,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_baudrate(FPM383C_UART_PORT, FPM383C_BAUD);
}





/*
查询手指有没有放置
1.发送指令【帧头】+【密码】+【命令0x01+0x35】+【校验】
2.等待模块回话，最多500ms
3.如果超时之前没有回话，返回错误
4.如果回应了但是夹带错误码，那么返回失败
5.检查回应的数据
 - 如果带1个字节的数据，是1，手指在位，resent改true/是0.手指不在位，present设false
 - 返回成功

*/


esp_err_t fpm383c_finger_present(bool *present)
{
    uint8_t data[10];
    int n;
    uint32_t err;

    if(present == NULL)
    return ESP_ERR_INVALID_ARG;

    fpm_send(0x01, 0x35, NULL, 0); // 发送
    uint8_t c1, c2;
    esp_err_t e= fpm_recv(&c1,&c2, &err, data, sizeof(data), &n, 500); // 接收
    if(e != ESP_OK) 
    {
        return e;
    }


    if (err != 0 || n < 1) 
        return ESP_FAIL;

         *present = (data[0] != 0); // 直接把 0/1 塞进指针
          return ESP_OK;

}



/*
同步匹配
将手指放置之后，将指纹与系统比对，
1.同步匹配函数，参数传递装结果的结构体指针result，传入最长等待事件timeout
2.等待模块回应，最多timeout
3.如果模块没有应答，返回错误
4.如果模块回了错误码，返回失败
5.解析回应：
回应6个字节，所以需要通过移位拼接
结果= 第0个字节<< 8 | 第一个字节
分数= 第2个字节<< 8 | 第3个字节
ID = 第4个字节 << 8 | 第5个字节

6.检查结果：
如果结果=1，true，保存ID，返回成功
否则，返回失败


1个字节等于8位，

*/


//匹配
esp_err_t fpm383c_match_sync(fpm383c_match_result_t *result, uint32_t timeout_ms)
{
    uint8_t c1,c2;

     uint8_t data[10];
    int n=0;
    uint32_t err=0;

    fpm_send(0x01, 0x23, NULL, 0); // 发送
    esp_err_t e=fpm_recv(&c1,&c2, &err, data, sizeof(data), &n, timeout_ms); // 接收

    if(e != ESP_OK) return 0;
    if (err != 0 || n < 6) return ESP_FAIL; // 不成功或数据不够6字节
    
    // 移位拼接：高字节在前，低字节在后
    //左移，1字节8位，所以移动8bit
    result->match_result = (data[0] << 8) | data[1];
    result->match_score  = (data[2] << 8) | data[3];
    result->match_id     = (data[4] << 8) | data[5];

    if (result->match_result == 0x0001) {
        return ESP_OK; // 成功匹配
    }
    return ESP_FAIL;

//把相邻的字节拼成完整数字，高字节<< 8 | 低字节



}























void fpm383c_test(void)
{
    fpm_send(0x01, 0x35, NULL, 0);   // 查询手指在位
    fpm383c_heartbeat();
}























