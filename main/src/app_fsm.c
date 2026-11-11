

 
#include "app_fsm.h"
#include "ui.h"
#include <string.h>
#include "lock_action.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define DEFAULT_PASSWORD "225566"
static char s_pw[16];

//fsm啥意思
//Finite State Machine 有限状态机，即收到哪一个按键跳到哪一个状态。页面

#define MENU_COUNT 5

static fsm_state_t s_state;
static int s_menu;
static int s_pw_len;

void FSM_Init(void)
{

    s_state = ST_MENU;
    s_menu = 0;
    s_pw_len = 0;//密码长度
    s_pw[0] = '\0';
    UI_Menu(s_menu);//显示菜单

}

//选中不同选项显示不同页面
static void enter_page(void)
{
    switch (s_menu) {
    case 0: s_state = ST_PASSWORD; s_pw_len = 0; UI_Password(s_pw_len); break;
    case 1: s_state = ST_FINGER;  UI_Finger();   break;
    case 2: s_state = ST_FACE;    UI_Face();     break;
    case 3: s_state = ST_RFID;    UI_RFID();     break;
    case 4: s_state = ST_DISTANCE; UI_Distance(); break;
    case 5: s_pw[0]= '\0';
    }
}

//返回主菜单页面
static void back_to_menu(void)
{
    s_state = ST_MENU;
    UI_Menu(s_menu);
}


// /按键输入
void FSM_HandleKey(char key)  // key: 'A' 'B' 'C' 'D' '0'~'9'
{
    switch (s_state) {
    case ST_MENU:
        if (key == 'A')
        { s_menu = (s_menu + MENU_COUNT - 1) % MENU_COUNT; UI_Menu(s_menu); }

    else if (key == 'B') { s_menu = (s_menu + 1) % MENU_COUNT; UI_Menu(s_menu); }
    else if (key == 'C') { enter_page(); }
        break;

    case ST_PASSWORD:
        if (key >= '0' && key <= '9') {
            if (s_pw_len < 15) {
        s_pw[s_pw_len++] = key;
        s_pw[s_pw_len] = '\0';
            }
            UI_Password(s_pw_len);
        } else if (key == 'D') {
           if (s_pw_len > 0) {
        s_pw_len--;
        s_pw[s_pw_len] = '\0';
    }
            UI_Password(s_pw_len);
            if (s_pw_len == 0) 
            back_to_menu();
        } else if (key == 'C') {
            // TODO: 真实密码校验，这里先模拟成功
            if (strcmp(s_pw, DEFAULT_PASSWORD) == 0) {
                printf("FSM: password OK\n");
                Lock_Open();
                back_to_menu();
            } else {
                printf("FSM: password WRONG\n");
                Lock_WrongPassword();
        s_pw_len = 0;
        s_pw[0] = '\0';
        UI_Password(0);
    }
        }
        break;

    case ST_FINGER:
        if (key == 'D') {
            back_to_menu();
        } else if (key == 'C') {
            // 现在按键模拟“识别成功”；之后指纹模块直接调 FSM_FingerResult
            FSM_FingerResult(true);
        }
        break;

    case ST_FACE:
        if (key == 'D') {
            back_to_menu();
        } else if (key == 'C') {
            FSM_FaceResult(true);
        }
        break;

    case ST_RFID:
        if (key == 'D') {
            back_to_menu();
        } else if (key == 'C') {
            FSM_RfidResult(true);
        }
        break;

    case ST_DISTANCE:
        if (key == 'D' || key == 'C') {
            back_to_menu();
        }
        break;

    case ST_UNLOCK:
        if (key == 'D') back_to_menu();
        break;

    default:
        if (key == 'D') back_to_menu();
        break;
    }
}


//识别成功的通用处理
static void on_verify_ok(void)
{
    Lock_Open();//开锁成功且返回菜单
    back_to_menu();
}

//识别失败，提示，延迟，返回菜单
static void on_verify_fail(const char *what)
{
    UI_Message(what, "TRY AGAIN");
    vTaskDelay(pdMS_TO_TICKS(1500));
    back_to_menu();
}


//指纹识别，不在返回，在指纹页面则通过开锁，失败提示
void FSM_FingerResult(bool ok)
{
    if (s_state != ST_FINGER) return;   // 不在指纹页就忽略
    if (ok) on_verify_ok();
    else on_verify_fail("FINGER FAIL");
}


void FSM_FaceResult(bool ok)
{
    if (s_state != ST_FACE) return;   // 不在人脸页就忽略
    if (ok) on_verify_ok();
    else on_verify_fail("Face FAIL");
}


void FSM_RfidResult(bool ok)
{
    if (s_state != ST_RFID) return;   // 不在刷卡页就忽略
    if (ok) on_verify_ok();
    else on_verify_fail("RFID FAIL");
}

