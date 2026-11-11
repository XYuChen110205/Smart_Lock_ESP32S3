


#include "ui.h"
#include "esp_lvgl_port.h"


void UI_Menu(int sel)
{
    static const char *items[] = {
        "PASSWORD",
        "FINGERPRINT",
        "FACE",
        "RFID",
        "INIT"
    };
    const int count = sizeof(items)/sizeof(items[0]);
    //在调用lvgl之前加锁
    lvgl_port_lock(0);

    //创建屏幕窗口
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);//不要上一页的内容残留
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);//设置背景颜色
    lv_obj_set_style_text_color(scr, lv_color_hex(0xFFFFFF), 0);//设置文本颜色

    //1.在创建的src屏幕内设置标题：：OCK
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "SMART LOCK");//在文本框内写入内容
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);//文本位置顶部居中

    //2.菜单，高亮（当前）
     for (int i = 0; i < count; i++) {
        lv_obj_t *lab = lv_label_create(scr);//创建文本框
        lv_label_set_text_fmt(lab, "%s%s", (i == sel) ? "> " : "  ", items[i]);
        //意思是这项选中输出>,否则输出空格
        //第二个%s，items[i]菜单名字

        lv_obj_set_style_text_color(lab,
            (i == sel) ? lv_color_hex(0xFFE000) : lv_color_hex(0xFFFFFF), 0);
            //左上角
            lv_obj_align(lab, LV_ALIGN_TOP_LEFT, 20, 50 + i * 34);//位置

        }

    //3.底部提示
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "A UP   B DOWN   C OK");
    
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -10);

    lvgl_port_unlock();//用完解锁







}


void UI_Password(int entered_len)
{
    //加锁
    lvgl_port_lock(0);

    //obj清除，创建屏幕，背景设置，文本颜色设置
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_color(scr, lv_color_hex(0xFFFFFF), 0);


    //创建控件，在屏幕创建text，设置text文本与位置

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "ENTER PASSWORD");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t *pw = lv_label_create(scr);
    if (entered_len == 0) {
        lv_label_set_text(pw, "_");
    } else {
        lv_label_set_text_fmt(pw, "%.*s", entered_len, "****************");
    //这一句为什么是%.*s,
    //只打印entered_len个字符，即从一串*中取前N个
    }

    //设置颜色样式位置
    lv_obj_set_style_text_color(pw, lv_color_hex(0xFFE000), 0);
    lv_obj_align(pw, LV_ALIGN_CENTER, 0, -20);

    //底部提示
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "C OK   D DEL");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -10);



    //解锁
    lvgl_port_unlock();

}


void UI_Message(const char *l1,const char *l2)
{
    //上锁
    lvgl_port_lock(0);

    //通用提示页面，文字
    //创建屏幕，清屏，设置背景颜色
    lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0);


    //标签设置文本， 在a页面放置l1内容
    lv_obj_t *a = lv_label_create(scr);
    lv_label_set_text(a, l1);
    lv_obj_set_style_text_color(a, lv_color_hex(0xF80000), 0); // 红
    lv_obj_align(a, LV_ALIGN_CENTER, 0, -30);

    //写入l2内容
    lv_obj_t *b = lv_label_create(scr);
    lv_label_set_text(b, l2);
    lv_obj_set_style_text_color(b, lv_color_hex(0xFFFFFF), 0); // 白
    lv_obj_align(b, LV_ALIGN_CENTER, 0, 20);


    //解锁
    lvgl_port_unlock();



}

//开锁页,如果是开锁页面，那么伴随着舵机转动角度
void UI_Unlock(void)
{
    //上锁
  lvgl_port_lock(0);

  //创建屏幕 清屏 背景
   lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x001F3F), 0);

    //设置标签 标签文本 文本颜色 位置
    lv_obj_t *t = lv_label_create(scr);
    lv_label_set_text(t, "UNLOCKED");
    lv_obj_set_style_text_color(t, lv_color_hex(0x00FF00), 0); // 绿
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);



    //解锁
    lvgl_port_unlock();

}




void UI_Finger(void)
{
    //上锁
  lvgl_port_lock(0);

  //创建屏幕 清屏 背景
   lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x001F3F), 0);

    //设置标签 标签文本 文本颜色 位置
    lv_obj_t *t = lv_label_create(scr);
    lv_label_set_text(t, "Place Finger");
    lv_obj_set_style_text_color(t, lv_color_hex(0x00FF00), 0); // 绿
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);



    //解锁
    lvgl_port_unlock();
}


void UI_Face(void)
{
    //上锁
  lvgl_port_lock(0);

  //创建屏幕 清屏 背景
   lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x001F3F), 0);

    //设置标签 标签文本 文本颜色 位置
    lv_obj_t *t = lv_label_create(scr);
    lv_label_set_text(t, "Look at there");
    lv_obj_set_style_text_color(t, lv_color_hex(0x00FF00), 0); // 绿
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);



    //解锁
    lvgl_port_unlock();

}


void UI_RFID(void)
{
    //上锁
  lvgl_port_lock(0);

  //创建屏幕 清屏 背景
   lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x001F3F), 0);

    //设置标签 标签文本 文本颜色 位置
    lv_obj_t *t = lv_label_create(scr);
    lv_label_set_text(t, "Tap RFID Card");
    lv_obj_set_style_text_color(t, lv_color_hex(0x00FF00), 0); // 绿
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);



    //解锁
    lvgl_port_unlock();

}


void UI_Distance(void)
{

    //上锁
  lvgl_port_lock(0);

  //创建屏幕 清屏 背景
   lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x001F3F), 0);

    //设置标签 标签文本 文本颜色 位置
    lv_obj_t *t = lv_label_create(scr);
    lv_label_set_text(t, "Distance Control");
    lv_obj_set_style_text_color(t, lv_color_hex(0x00FF00), 0); // 绿
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);



    //解锁
    lvgl_port_unlock();
}


void UI_Remote(void)
{
    //上锁
  lvgl_port_lock(0);

  //创建屏幕 清屏 背景
   lv_obj_t *scr = lv_screen_active();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x001F3F), 0);

    //设置标签 标签文本 文本颜色 位置
    lv_obj_t *t = lv_label_create(scr);
    lv_label_set_text(t, "Remote Control");
    lv_obj_set_style_text_color(t, lv_color_hex(0x00FF00), 0); // 绿
    lv_obj_align(t, LV_ALIGN_CENTER, 0, 0);



    //解锁
    lvgl_port_unlock();

}









