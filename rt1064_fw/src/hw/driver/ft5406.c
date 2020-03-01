/*
 * ft5406.c
 *
 *  Created on: 2020. 3. 1.
 *      Author: Baram
 */




#include "ft5406.h"

#ifdef _USE_HW_FT5406
#include "i2c.h"
#include "cmdif.h"



#define FT5406_I2C_ADDR             0x38
#define FT5406_MAX_TOUCHES          5

#define FT5406_REG_DEVICE_MODE      0x00
#define FT5406_REG_GEST_ID          0x01
#define FT5406_REG_TD_STATUS        0x02
#define FT5406_REG_TOUCH1_XH        0x03

#define FT5406_ID_G_CHIPID          0xA3


static uint8_t i2c_ch = _DEF_I2C1;
static bool is_init = false;

#ifdef _USE_HW_RTOS
static osMutexId mutex_id;
#endif


typedef struct
{
    uint8_t XH;
    uint8_t XL;
    uint8_t YH;
    uint8_t YL;
    uint8_t RESERVED[2];
} ft5406_touch_point_t;


#ifdef _USE_HW_CMDIF
void ft5406Cmdif(void);
#endif

static bool ft5406ReadReg(uint16_t addr, uint8_t *p_data, uint32_t length);
static bool ft5406WriteReg(uint16_t addr, uint8_t *p_data, uint32_t length);



bool ft5406Init(void)
{
  uint8_t data = 0;


  if (i2cIsInit() != true) return false;

  if (i2cIsBegin(i2c_ch) != true)
  {
    i2cBegin(i2c_ch, 400);
  }

  logPrintf("sdCard     \t\t: connected\r\n");


  if (ft5406ReadReg(FT5406_ID_G_CHIPID, &data, 1) == true)
  {
    logPrintf("ft5406 id  \t\t: 0x%02X\r\n", data);
  }
  else
  {
    logPrintf("ft5406 id  \t\t: Fail\r\n");
  }

  data = 0;
  ft5406WriteReg(FT5406_REG_DEVICE_MODE, &data, 1);


  is_init = true;

#ifdef _USE_HW_RTOS
  osMutexDef(mutex_id);
  mutex_id = osMutexCreate (osMutex(mutex_id));
#endif

#ifdef _USE_HW_CMDIF
  cmdifAdd("ft5406", ft5406Cmdif);
#endif
  return true;
}

uint8_t ft5406GetGestureID(void)
{
  uint8_t ret = 0;
  uint8_t data;

#ifdef _USE_HW_RTOS
  osMutexWait(mutex_id, osWaitForever);
#endif

  if (ft5406ReadReg(FT5406_REG_GEST_ID, &data, 1) == true)
  {
    ret = data;
  }

#ifdef _USE_HW_RTOS
  osMutexRelease(mutex_id);
#endif

  return ret;
}

uint8_t ft5406GetTouchedCount(void)
{
  uint8_t ret = 0;
  uint8_t data;

#ifdef _USE_HW_RTOS
  osMutexWait(mutex_id, osWaitForever);
#endif

  if (ft5406ReadReg(FT5406_REG_TD_STATUS, &data, 1) == true)
  {
    ret = data & 0x0F;

    if (ret > FT5406_MAX_TOUCHES)
    {
      ret = 0;
    }
  }

#ifdef _USE_HW_RTOS
  osMutexRelease(mutex_id);
#endif
  return ret;
}

bool ft5406GetTouchedData(uint8_t index, ft5406_data_t *p_data)
{
  bool ret = false;
  ft5406_touch_point_t data;

  if (index >= FT5406_MAX_TOUCHES)
  {
    return false;
  }

#ifdef _USE_HW_RTOS
  osMutexWait(mutex_id, osWaitForever);
#endif

  if (ft5406ReadReg(FT5406_REG_TOUCH1_XH + index*6, (uint8_t *)&data, 6) == true)
  {
    p_data->event = data.XH >> 6;
    p_data->id    = data.YH >> 4;

    p_data->y  = data.XL;
    p_data->y |= (data.XH & 0x0F) << 8;
    p_data->x  = data.YL;
    p_data->x |= (data.YH & 0x0F) << 8;

    ret = true;
  }

#ifdef _USE_HW_RTOS
  osMutexRelease(mutex_id);
#endif
  return ret;
}


bool ft5406ReadReg(uint16_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret;

  ret = i2cReadBytes(i2c_ch, FT5406_I2C_ADDR, addr, p_data, length, 10);

  return ret;
}

bool ft5406WriteReg(uint16_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret;

  ret = i2cWriteBytes(i2c_ch, FT5406_I2C_ADDR, addr, p_data, length, 10);

  return ret;
}





#ifdef _USE_HW_CMDIF
void ft5406Cmdif(void)
{
  bool ret = true;


  if (cmdifGetParamCnt() == 1  && cmdifHasString("info", 0) == true)
  {
    uint8_t touch_cnt;
    ft5406_data_t touch_data;;

    while(cmdifRxAvailable() == 0)
    {
      touch_cnt = ft5406GetTouchedCount();
      if ( touch_cnt> 0)
      {
        cmdifPrintf("Touch : %d, ", touch_cnt);

        for (int i=0; i<touch_cnt; i++)
        {
          ft5406GetTouchedData(i, &touch_data);
          cmdifPrintf("[%d %d, x=%03d y=%03d] ", touch_data.event, touch_data.id, touch_data.x, touch_data.y);
        }
        cmdifPrintf("\n");
      }
      delay(10);
    }
  }
  else
  {
    ret = false;
  }


  if (ret == false)
  {
    cmdifPrintf( "ft5406 info \n");
  }
}
#endif


#endif //#ifdef _USE_HW_FT5406
