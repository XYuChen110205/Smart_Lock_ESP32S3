
#ifndef __UI_H__
#define __UI_H__

typedef enum{
    PAGE_MENU,
    PAGE_PASSWORD,
    PAGE_FINGER,
    PAGE_FACE,
    PAGE_RFID,
    PAGE_DISTANCE,
    PAGE_UNLOCK,
    PAGE_MESSAGE,
    PAGE_REMOTE
}page_t;

void UI_Menu(int sel);//当前高亮项

void UI_Password(int entered_len);
void UI_Message(const char *l1,const char *l2);
void UI_Unlock(void);

void UI_Finger(void);
void UI_Face(void);
void UI_RFID(void);
void UI_Distance(void);
void UI_Remote(void);




#endif

