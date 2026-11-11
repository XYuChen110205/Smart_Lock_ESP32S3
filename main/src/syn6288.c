
#include "syn6288.h"
#include "driver/uart.h"
#include "string.h"
#include "pins.h"


static const char *TAG = "syn6288";


//1.uart的初始化

esp_err_t Syn6288_Init(void) {
    uart_config_t cfg = {
        .baud_rate = 9600, // 手册写的初始波特率9600
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

//安装驱动 + 绑定引脚
 uart_driver_install(SYN_UART_PORT, 1024, 0, 0, NULL, 0);
    uart_param_config(SYN_UART_PORT, &cfg);
    // 绑定引脚，注意 RX 引脚如果填 -1 可能会报错，所以随便填个没用的，比如 15
    uart_set_pin(SYN_UART_PORT, SYN_TX_PIN, SYN_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    return ESP_OK;
}




//2.语音播放业务
void Syn6288_PlayText(const char *text) {
    int text_len = strlen(text);//输入的文本
    uint8_t frame[206];//手册最大206

    //1.帧头
    frame[0] = 0xFD;

    //2.数据区常速= 命令字节（1）+命令参数（1）+待发送文本（N）+异或校验（1）=N+3
    uint16_t data_len = text_len + 3;
    frame[1] = (data_len >> 8) & 0xFF;//高字节
    frame[2] = data_len & 0xFF;//低字节    为什么第一个不是低字节我搞不懂是先写进去所以需要移动进去吗

    //3.命令字+编码 1字节
    frame[3] = 0x01;//01是语音合成播放
    frame[4] = 0x00;//采用GB2312编码

    //4.填入文本
    memcpy(frame + 5,text,text_len);//通过memcpy的方式将一串内容写入数组buffer

    //5.异或校验 1 字节
    uint8_t xor_sum = 0;
    int total_len = 5+text_len;  //计算总长度，从0xFD到最后一个字节
    for(int i = 0; i < total_len ; i ++)
    {
        xor_sum ^= frame[i];
    }

    frame[total_len] = xor_sum; //把校验和塞到最后的位置

    //6.发送数据（总长度= 帧头1 + 长度2 + 数据区len+校验和=5+N+1 = 6+N
    uart_write_bytes(SYN_UART_PORT, frame, 6 + text_len);//通过串口SYN发送frame，长度是。。。
}

    

    











