/*
 * hw.c
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */




#include "hw.h"




extern uint32_t __vectors_start__;
extern uint32_t _image_start;
extern uint32_t _image_size;


__attribute__((section(".tag"))) const flash_tag_t fw_tag =
   {
    // fw info
    //
    0xAAAA5555,        // magic_number
    "V200229R1",       // version_str
    "RT1064_B/D",      // board_str
    "Firmware",        // name
    __DATE__,
    __TIME__,
    (uint32_t)&fw_tag,
    (uint32_t)&__vectors_start__,

    (uint32_t)&_image_start,  // load_addr
    (uint32_t)&_image_size,   // load_size


    // tag info
    //
   };


void hwInit(void)
{
  bspInit();

  microsInit();
  swtimerInit();
  cmdifInit();

  resetInit();
  flashInit();

  ledInit();
  buttonInit();
  i2cInit();
  uartInit();
  uartOpen(_DEF_UART1, 57600);

  logPrintf("\n\n[ Firmware Begin... ]\r\n");
  logPrintf("Addr Tag   \t\t: 0x%X\r\n", (int)fw_tag.addr_tag);
  logPrintf("Addr Fw    \t\t: 0x%X\r\n", (int)fw_tag.addr_fw);
  logPrintf("Addr Hw    \t\t: 0x%X\r\n", (int)hwInit);

  resetLog();
  clocksInit();

  if (fw_tag.addr_fw == fw_tag.load_start)
  {
    sdramInit();
  }
  gpioInit();

  if (sdInit() == true)
  {
    fatfsInit();
  }

  lcdInit();
  ft5406Init();

  usbdInit();
}
