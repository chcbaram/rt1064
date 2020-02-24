/*
 * hw.c
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */




#include "hw.h"



__attribute__((aligned(2048))) __attribute__((used, section(".tag"))) const uint8_t boot_name[32] = "RT1064_B/D";
                               __attribute__((used, section(".tag"))) const uint8_t boot_ver[32]  = "B200224R1";


void hwInit(void)
{
  bspInit();

  microsInit();
  swtimerInit();
  cmdifInit();
  flashInit();

  ledInit();
  buttonInit();
  uartInit();
  uartOpen(_DEF_UART1, 57600);

  logPrintf("\n\n[ Bootloader Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", boot_name);
  logPrintf("Booting..Ver  \t\t: %s\r\n", boot_ver);

  logPrintf("Tag Addr   \t\t: 0x%X\r\n", (int)&boot_name[0]);

  clocksInit();
  sdramInit();
  gpioInit();



  vcpInit();
}
