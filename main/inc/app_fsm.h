

#ifndef __APP_FSM_H__
#define __APP_FSM_H__

#include <stdbool.h>

typedef enum{
    ST_MENU,
    ST_PASSWORD,
    ST_FINGER,
    ST_FACE,
    ST_RFID,
    ST_DISTANCE,
    ST_UNLOCK,
    ST_MESSAGE,
    ST_REMOTE,
} fsm_state_t;

void FSM_Init(void);
void FSM_HandleKey(char key);   // key: 'A' 'B' 'C' 'D' '0'~'9'

void FSM_FingerResult(bool ok);   // 指纹结果
void FSM_FaceResult(bool ok);     // 人脸结果
void FSM_RfidResult(bool ok);     // 刷卡结果

#endif
