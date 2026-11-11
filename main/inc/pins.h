

#ifndef __PINS_H__
#define __PINS_H__


#include "driver/uart.h"
#include "driver/spi_master.h"


//1.ST7789屏幕
#define LCD_HOST SPI2_HOST
#define LCD_SCLK 12
#define LCD_MOSI 11
#define LCD_MISO 18 //与rc522使用同一个，但是lcd这里用不上miso
#define LCD_CS 13
#define LCD_DC 9
#define LCD_RST 10
#define LCD_BLK 14


#define LCD_H_RES 240
#define LCD_V_RES 240
#define LCD_BK_ON_LEVEL 1


//2.MDRC522射频卡
#define RFID_CS 21
#define RFID_RST 17

//3.FPM383C指纹模块，使用UART1
#define FP_UART_PORT UART_NUM_1
#define FP_TX 15
#define FP_RX 16
#define FP_TOUCH 5
#define FP_BAUD 57600

//4.人脸模块FM225，使用UART2
#define FACE_UART_PORT UART_NUM_2
#define FM_TX 2 //主控发送到模块的rx交叉
#define FM_RX 1
#define FM_BAUD 115200

//5.SYN6288 TTS语音模块，使用UART2，同FM225
#define TTS_UART_PORT UART_NUM_2
#define TTS_TX 6 //GIO6接模块RXD
#define TTS_RX 42
#define TTS_BAUD 9600


//6.PCF8574键盘，使用I2C，通过扩展口
#define KEY_I2C_SDA 8
#define KEY_I2C_SCL 41

//7.舵机和buzzer
#define SERVO_GPIO 7
#define BUZZER_GPIO 40
#define VIBRATION_GPIO 39






#endif

