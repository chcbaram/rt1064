/*
 * hw.c
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */




#include "hw.h"



__attribute__((aligned(2048))) __attribute__((used, section(".tag"))) const uint8_t boot_name[32] = "RT1064_B/D";
                               __attribute__((used, section(".tag"))) const uint8_t boot_ver[32]  = "V200223R1";


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
  logPrintf("Tag Addr   \t\t: 0x%X\r\n", (int)&boot_name[0]);

  clocksInit();
  sdramInit();
  gpioInit();

  if (sdInit() == true)
  {
    fatfsInit();
  }

  lcdInit();
}
