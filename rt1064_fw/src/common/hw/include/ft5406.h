/*
 * ft5406.h
 *
 *  Created on: 2020. 3. 1.
 *      Author: Baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_FT5406_H_
#define SRC_COMMON_HW_INCLUDE_FT5406_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"

#ifdef _USE_HW_FT5406



typedef struct
{
  uint8_t  event;
  uint8_t  id;
  int16_t  x;
  int16_t  y;
} ft5406_data_t;

bool    ft5406Init(void);
uint8_t ft5406GetTouchedCount(void);
bool    ft5406GetTouchedData(uint8_t index, ft5406_data_t *p_data);
#endif

#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_FT5406_H_ */
