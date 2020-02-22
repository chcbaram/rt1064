/*
 * ap.cpp
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */




#include "ap.h"



void apInit(void)
{
  hwInit();

  cmdifOpen(_DEF_UART1, 57600);
  uartOpen(_DEF_UART2, 57600);
}


void apMain(void)
{
  uint32_t pre_time;
  uint16_t x = 0;
  uint16_t y = 0;

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
      uartPrintf(_DEF_UART2, "rx : 0x%X \n", uartRead(_DEF_UART2));

#if 0
      uint32_t pre_time;

      pre_time = millis();
      uint8_t *p_buf = (uint8_t *)0x70000000;
      volatile uint32_t data = 0;
      for (int i=0; i<4*1024*1024; i++)
      {
        data += p_buf[i];
      }
      logPrintf("%d ms\n", millis() - pre_time);
#endif
    }


    static uint32_t lcd_pre_time;
    static uint32_t fps_time;
    static uint32_t fps;

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

  }
}
