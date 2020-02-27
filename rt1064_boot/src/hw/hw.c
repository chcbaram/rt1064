/*
 * hw.c
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */




#include "hw.h"



__attribute__((aligned(2048))) __attribute__((used, section(".tag"))) const uint8_t boot_name[32] = "RT1064_B/D";
                               __attribute__((used, section(".tag"))) const uint8_t boot_ver[32]  = "B200227R1";


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
  uartInit();
  uartOpen(_DEF_UART1, 57600);

  logPrintf("\n\n[ Bootloader Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", boot_name);
  logPrintf("Booting..Ver  \t\t: %s\r\n", boot_ver);

  logPrintf("Tag Addr   \t\t: 0x%X\r\n", (int)&boot_name[0]);
  logPrintf("Boot Mode  \t\t: 0x%X\r\n", (int)resetGetBootMode());
  resetLog();
  clocksInit();
  sdramInit();
  gpioInit();



  if (sdInit() == true)
  {
    fatfsInit();
  }

  if (resetGetBootMode() & (1<<0))
  {
    flash_tag_t *p_tag = (flash_tag_t *)FLASH_ADDR_TAG;
    void (**jump_func)(void) = (void (**)(void))(p_tag->addr_fw + 4);


    resetSetBootMode(0);


    if (p_tag->magic_number == 0x5555AAAA || p_tag->magic_number == 0xAAAA5555)
    {
      if (p_tag->addr_fw != p_tag->load_start)
      {
        uint32_t pre_time;
        pre_time = micros();
        memcpy((void *)p_tag->addr_fw, (const void *)p_tag->load_start, p_tag->load_size);
        SCB_InvalidateDCache_by_Addr((void *)(p_tag->addr_fw), p_tag->load_size);
        cmdifPrintf("Load Fw   : %d ms \n", (micros()-pre_time)/1000);
      }

      cmdifPrintf("Jump Addr : 0x%X \n", (int)(*jump_func));

      delay(100);
      bspDeInit();

      __set_MSP(*(uint32_t *)p_tag->addr_fw);
      (*jump_func)();
    }
  }

  usbdInit();
}
