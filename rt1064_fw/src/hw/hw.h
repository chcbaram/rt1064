/*
 * hw.h
 *
 *  Created on: 2020. 2. 17.
 *      Author: Baram
 */

#ifndef SRC_HW_HW_H_
#define SRC_HW_HW_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"


#include "led.h"
#include "uart.h"
#include "cmdif.h"
#include "swtimer.h"
#include "button.h"
#include "vcp.h"
#include "flash.h"
#include "clocks.h"
#include "sdram.h"
#include "gpio.h"
#include "sd.h"
#include "micros.h"
#include "lcd.h"
#include "usbd.h"
#include "reset.h"
#include "i2c.h"
#include "ft5406.h"


#include "fatfs/fatfs.h"

void hwInit(void);


#ifdef __cplusplus
}
#endif

#endif /* SRC_HW_HW_H_ */
