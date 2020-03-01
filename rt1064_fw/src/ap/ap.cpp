/*
 * ap.cpp
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */




#include "ap.h"
#include "LittlevGL/LittlevGL.h"


static void threadEmul(void const *argument);



void apInit(void)
{
  hwInit();

  cmdifOpen(_DEF_UART1, 57600);
  uartOpen(_DEF_UART2, 57600);

  osThreadDef(threadEmul, threadEmul, _HW_DEF_RTOS_THREAD_PRI_EMUL, 0, _HW_DEF_RTOS_THREAD_MEM_EMUL);
  if (osThreadCreate(osThread(threadEmul), NULL) != NULL)
  {
    logPrintf("threadEmul \t\t: OK\r\n");
  }
  else
  {
    logPrintf("threadEmul \t\t: Fail\r\n");
    while(1);
  }
}


void apMain(void)
{
  uint32_t pre_time;


  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
    }

    cmdifMain();

    osThreadYield();
  }
}


static void threadEmul(void const *argument)
{
  uint32_t lcd_pre_time;
  uint32_t fps_time;
  uint32_t fps;
  uint16_t x = 0;
  uint16_t y = 0;


  LittlevGL();

  while(1)
  {
    lcdMutexWait();

    if (lcdDrawAvailable() > 0)
    {
      lcdClearBuffer(black);

      lcdPrintf(150, 0, white, "i.MX RT1064 보드");

      if (millis()-lcd_pre_time >= 1000)
      {
        lcd_pre_time = millis();
        fps_time = lcdGetFpsTime();
        fps = lcdGetFps();
      }

      lcdPrintf(0,  0, white, "%d ms", fps_time);
      lcdPrintf(0, 16, white, "%d fps", fps);

      lcdDrawFillRect(x, 32, 30, 30, red);
      lcdDrawFillRect(lcdGetWidth()-x, 62, 30, 30, green);
      lcdDrawFillRect(x + 30, 92, 30, 30, blue);

      x += 4;

      x %= lcdGetWidth();
      y %= lcdGetHeight();

#if 1
      uint8_t touch_cnt;
      ft5406_data_t touch_data;;


      touch_cnt = ft5406GetTouchedCount();
      if ( touch_cnt> 0)
      {
        for (int i=0; i<touch_cnt; i++)
        {
          ft5406GetTouchedData(i, &touch_data);

          lcdPrintf(touch_data.x-30, touch_data.y-30-16, white, "%d %d %d", touch_data.id, touch_data.x, touch_data.y);
          lcdDrawFillRect(touch_data.x-30, touch_data.y-30, 60, 60, green);
        }
      }
#endif
      lcdRequestDraw();
    }

    lcdMutexRelease();

    osThreadYield();
  }
}
