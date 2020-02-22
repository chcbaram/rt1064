/*
 * hw.c
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */




#include "hw.h"





void hwInit(void)
{
  bspInit();

  microsInit();
  swtimerInit();
  cmdifInit();
  flashInit();

  ledInit();
  buttonInit();
  vcpInit();
  uartInit();
  uartOpen(_DEF_UART1, 57600);

  logPrintf("\n\n[ Firmware Begin... ]\r\n");


  clocksInit();
  sdramInit();
  gpioInit();

  if (sdInit() == true)
  {
    fatfsInit();
  }

  lcdInit();
}
