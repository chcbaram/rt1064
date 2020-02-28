/*
 * ap.cpp
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */




#include "ap.h"



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

    while (uartAvailable(_DEF_UART2) > 0)
    {
      uartPrintf(_DEF_UART1, "rx : 0x%X \n", uartRead(_DEF_UART2));
    }

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


  while(1)
  {
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

      lcdRequestDraw();
    }
    osThreadYield();
  }
}
