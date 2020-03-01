/*
 * LittlevGL.c
 *
 *  Created on: 2020. 3. 1.
 *      Author: Baram
 */




#include "LittlevGL.h"
#include "lcd.h"
#include "cmdif.h"

#include "lvgl.h"
#include "lvgl/porting/lv_port_disp.h"
#include "lvgl/porting/lv_port_indev.h"

#include "lv_test_theme/lv_test_theme_1.h"
#include "lv_test_theme/lv_test_theme_2.h"


static lv_theme_t *p_theme = NULL;


void lvglCmdif(void);


void LittlevGL(void)
{
  lv_init();
  lv_port_disp_init();
  lv_port_indev_init();

  cmdifAdd("lvgl", lvglCmdif);
}

void LittlevGLMain(void)
{
  lv_task_handler();
  delay(5);
}






void lv_ex_tabview_1(void)
{
    /*Create a Tab view object*/
    lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), NULL);

    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tab 1");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Tab 2");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Tab 3");


    /*Add content to the tabs*/
    lv_obj_t * label = lv_label_create(tab1, NULL);
    lv_label_set_text(label, "This the first tab\n\n"
                             "If the content\n"
                             "of a tab\n"
                             "become too long\n"
                             "the it \n"
                             "automatically\n"
                             "become\n"
                             "scrollable.");

    label = lv_label_create(tab2, NULL);
    lv_label_set_text(label, "Second tab");

    label = lv_label_create(tab3, NULL);
    lv_label_set_text(label, "Third tab");
}



void lvglCmdif(void)
{
  bool ret = true;



  if (cmdifGetParamCnt() == 1 && cmdifHasString("run", 0) == true)
  {

    if (p_theme == NULL)
    {
      //p_theme = lv_theme_default_init(0, NULL);
      p_theme = lv_theme_night_init(0, NULL);
    }


    lcdMutexWait();

    lv_port_disp_init();
    lv_port_indev_init();



    lv_test_theme_1(p_theme);
    //lv_test_theme_2();
    //lv_ex_tabview_1();

    while(cmdifRxAvailable() == 0)
    {
      LittlevGLMain();
    }
    lcdMutexRelease();
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "lvgl run \n");
  }
}


