/*
 * i2c.c
 *
 *  Created on: 2020. 3. 1.
 *      Author: Baram
 */




#include "i2c.h"
#include "cmdif.h"
#include "fsl_lpi2c.h"


#ifdef _USE_HW_I2C

#ifdef _USE_HW_CMDIF
static void i2cCmdifInit(void);
static void i2cCmdif(void);
#endif



static uint32_t i2c_timeout[I2C_MAX_CH];
static uint32_t i2c_errcount[I2C_MAX_CH];
static uint32_t i2c_freq[I2C_MAX_CH];

static bool is_init = false;
static bool is_begin[I2C_MAX_CH];




typedef struct
{
  LPI2C_Type             *p_hi2c;
  lpi2c_master_transfer_t xfer;

} i2c_tbl_t;

static i2c_tbl_t i2c_tbl[I2C_MAX_CH] =
    {
        {LPI2C1,},
    };


bool i2cInit(void)
{
  uint32_t i;


  for (i=0; i<I2C_MAX_CH; i++)
  {
    i2c_timeout[i] = 10;
    i2c_errcount[i] = 0;
    is_begin[i] = false;
  }

#ifdef _USE_HW_CMDIF
  i2cCmdifInit();
#endif

  is_init = true;
  return true;
}

bool i2cIsInit(void)
{
  return is_init;
}

bool i2cBegin(uint8_t ch, uint32_t freq_khz)
{
  bool ret = false;
  lpi2c_master_config_t masterConfig = {0};
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }


  switch(ch)
  {
    case _DEF_I2C1:
      i2c_freq[ch] = freq_khz;

      /*
       * masterConfig.debugEnable = false;
       * masterConfig.ignoreAck = false;
       * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
       * masterConfig.baudRate_Hz = 100000U;
       * masterConfig.busIdleTimeout_ns = 0;
       * masterConfig.pinLowTimeout_ns = 0;
       * masterConfig.sdaGlitchFilterWidth_ns = 0;
       * masterConfig.sclGlitchFilterWidth_ns = 0;
       */
      LPI2C_MasterGetDefaultConfig(&masterConfig);


      masterConfig.baudRate_Hz = freq_khz * 1000;

      LPI2C_MasterInit(p_handle, &masterConfig, ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (CLOCK_GetDiv(kCLOCK_Lpi2cDiv) + 1U)));

      ret = true;
      is_begin[ch] = true;
      break;
  }

  return ret;
}

bool i2cIsBegin(uint8_t ch)
{
  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  return is_begin[ch];
}

void i2cReset(uint8_t ch)
{
}

bool i2cIsDeviceReady(uint8_t ch, uint8_t dev_addr)
{
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;
  status_t i2c_ret;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  /*
  mode                 = 0;
  xfer->slaveAddress   = FT5406_RT_I2C_ADDRESS;
  xfer->direction      = kLPI2C_Write;
  xfer->subaddress     = 0;
  xfer->subaddressSize = 1;
  xfer->data           = &mode;
  xfer->dataSize       = 1;
  xfer->flags          = kLPI2C_TransferDefaultFlag;
  */

  i2c_ret = LPI2C_MasterStart(p_handle, dev_addr, kLPI2C_Write);
  delay(2);
  i2c_ret = LPI2C_MasterStop(p_handle);

  if (i2c_ret == kStatus_Success)
  {
    return true;
  }

  return false;
}

bool i2cRecovery(uint8_t ch)
{
  bool ret;

  i2cReset(ch);

  ret = i2cBegin(ch, i2c_freq[ch]);

  return ret;
}

bool i2cReadByte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t timeout)
{
  return i2cReadBytes(ch, dev_addr, reg_addr, p_data, 1, timeout);
}

bool i2cReadByte2 (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t timeout)
{
  bool ret = false;
#if 0
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }


  if (HAL_I2C_Master_Transmit(p_handle, (uint16_t)(dev_addr << 1), (uint8_t *)&reg_addr, 1, timeout) == HAL_OK)
  {
    if (HAL_I2C_Master_Receive(p_handle, (uint16_t)(dev_addr << 1), p_data, 1, timeout) == HAL_OK)
    {
      ret = true;
    }
  }
  else
  {
    ret = false;
  }
#endif

  return ret;
}

bool i2cReadBytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret;
  status_t i2c_ret;
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;
  lpi2c_master_transfer_t *xfer = &(i2c_tbl[ch].xfer);

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  xfer->slaveAddress   = dev_addr;
  xfer->direction      = kLPI2C_Read;
  xfer->subaddress     = reg_addr;
  xfer->subaddressSize = 1;
  xfer->data           = p_data;
  xfer->dataSize       = length;
  xfer->flags          = kLPI2C_TransferDefaultFlag;

  i2c_ret = LPI2C_MasterTransferBlocking(p_handle, xfer);

  if (i2c_ret == kStatus_Success)
  {
    ret = true;
  }
  else
  {
    LPI2C_MasterStop(p_handle);
    ret = false;
  }

  return ret;
}

bool i2cRead16Byte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t timeout)
{
  return i2cRead16Bytes(ch, dev_addr, reg_addr, p_data, 1, timeout);
}

bool i2cRead16Bytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
#if 0
  status_t i2c_ret;
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  i2c_ret = HAL_I2C_Mem_Read(p_handle, (uint16_t)(dev_addr << 1), reg_addr, I2C_MEMADD_SIZE_16BIT, p_data, length, timeout);

  if( i2c_ret == HAL_OK )
  {
    ret = true;
  }
  else
  {
    ret = false;
  }
#endif

  return ret;
}

bool i2cReadData(uint8_t ch, uint16_t dev_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
#if 0
  status_t i2c_ret;
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  i2c_ret = HAL_I2C_Master_Receive(p_handle, (uint16_t)(dev_addr << 1), p_data, length, timeout);

  if( i2c_ret == HAL_OK )
  {
    ret = true;
  }
  else
  {
    ret = false;
  }
#endif
  return ret;
}

bool i2cWriteByte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t data, uint32_t timeout)
{
  return i2cWriteBytes(ch, dev_addr, reg_addr, &data, 1, timeout);
}

bool i2cWriteByte2(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t data, uint32_t timeout)
{
  bool ret = false;
#if 0
  status_t i2c_ret;
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;
  uint8_t buf[] = {reg_addr, data};

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  i2c_ret = HAL_I2C_Master_Transmit(p_handle, (uint16_t)(dev_addr << 1), buf, 2, timeout);

  if(i2c_ret == HAL_OK)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }
#endif

  return ret;
}

bool i2cWriteBytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret;
  status_t i2c_ret;
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;
  lpi2c_master_transfer_t *xfer = &(i2c_tbl[ch].xfer);


  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  xfer->slaveAddress   = dev_addr;
  xfer->direction      = kLPI2C_Write;
  xfer->subaddress     = reg_addr;
  xfer->subaddressSize = 1;
  xfer->data           = p_data;
  xfer->dataSize       = length;
  xfer->flags          = kLPI2C_TransferDefaultFlag;

  i2c_ret = LPI2C_MasterTransferBlocking(p_handle, xfer);

  if (i2c_ret == kStatus_Success)
  {
    ret = true;
  }
  else
  {
    LPI2C_MasterStop(p_handle);
    ret = false;
  }

  return ret;
}

bool i2cWrite16Byte (uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t data, uint32_t timeout)
{
  return i2cWrite16Bytes(ch, dev_addr, reg_addr, &data, 1, timeout);
}

bool i2cWrite16Bytes(uint8_t ch, uint16_t dev_addr, uint16_t reg_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
#if 0
  status_t i2c_ret;
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  i2c_ret = HAL_I2C_Mem_Write(p_handle, (uint16_t)(dev_addr << 1), reg_addr, I2C_MEMADD_SIZE_16BIT, p_data, length, timeout);

  if(i2c_ret == HAL_OK)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }
#endif
  return ret;
}
bool i2cWriteData(uint8_t ch, uint16_t dev_addr, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
  bool ret = false;
#if 0
  status_t i2c_ret;
  LPI2C_Type *p_handle = i2c_tbl[ch].p_hi2c;

  if (ch >= I2C_MAX_CH)
  {
    return false;
  }

  i2c_ret = HAL_I2C_Master_Transmit(p_handle, (uint16_t)(dev_addr << 1), p_data, length, timeout);

  if(i2c_ret == HAL_OK)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }
#endif
  return ret;
}

void i2cSetTimeout(uint8_t ch, uint32_t timeout)
{
  i2c_timeout[ch] = timeout;
}

uint32_t i2cGetTimeout(uint8_t ch)
{
  return i2c_timeout[ch];
}

void i2cClearErrCount(uint8_t ch)
{
  i2c_errcount[ch] = 0;
}

uint32_t i2cGetErrCount(uint8_t ch)
{
  return i2c_errcount[ch];
}







#ifdef _USE_HW_CMDIF
void i2cCmdifInit(void)
{
  cmdifAdd("i2c", i2cCmdif);
}


void i2cCmdif(void)
{
  bool ret = true;
  bool i2c_ret;

  uint8_t print_ch;
  uint8_t ch;
  uint16_t dev_addr;
  uint16_t reg_addr;
  uint16_t length;

  uint32_t i;
  uint8_t i2c_data[128];
  uint32_t pre_time;


  if (cmdifGetParamCnt() == 2)
  {
    print_ch = (uint16_t) cmdifGetParam(1);

    print_ch = constrain(print_ch, 1, I2C_MAX_CH);
    print_ch -= 1;

    if(cmdifHasString("scan", 0) == true)
    {
      int dev_cnt = 0;
      for (i=0x00; i<= 0x7F; i++)
      {
        if (i2cIsDeviceReady(print_ch, i) == true)
        {
          cmdifPrintf("I2C CH%d Addr 0x%02X : OK\n", print_ch+1, i);
          dev_cnt++;
        }
      }
      cmdifPrintf("I2C CH%d Device Found %d \n", print_ch+1, dev_cnt);
    }
    else if(cmdifHasString("begin", 0) == true)
    {
      i2c_ret = i2cBegin(print_ch, 400);
      if (i2c_ret == true)
      {
        cmdifPrintf("I2C CH%d Begin OK\n", print_ch + 1);
      }
      else
      {
        cmdifPrintf("I2C CH%d Begin Fail\n", print_ch + 1);
      }
    }
  }
  else if (cmdifGetParamCnt() == 5)
  {
    print_ch = (uint16_t) cmdifGetParam(1);
    print_ch = constrain(print_ch, 1, I2C_MAX_CH);

    dev_addr = (uint16_t) cmdifGetParam(2);
    reg_addr = (uint16_t) cmdifGetParam(3);
    length   = (uint16_t) cmdifGetParam(4);
    ch       = print_ch - 1;

    if(cmdifHasString("read", 0) == true)
    {
      for (i=0; i<length; i++)
      {
        i2c_ret = i2cReadByte(ch, dev_addr, reg_addr+i, i2c_data, 100);

        if (i2c_ret == true)
        {
          cmdifPrintf("%d I2C - 0x%02X : 0x%02X\n", print_ch, reg_addr+i, i2c_data[0]);
        }
        else
        {
          cmdifPrintf("%d I2C - Fail \n", print_ch);
          break;
        }
      }
    }
    else if(cmdifHasString("read2", 0) == true)
    {
      for (i=0; i<length; i++)
      {
        i2c_ret = i2cReadByte2(ch, dev_addr, reg_addr+i, i2c_data, 100);

        if (i2c_ret == true)
        {
          cmdifPrintf("%d I2C - 0x%02X : 0x%02X\n", print_ch, reg_addr+i, i2c_data[0]);
        }
        else
        {
          cmdifPrintf("%d I2C - Fail \n", print_ch);
          break;
        }
      }
    }
    else if(cmdifHasString("read16", 0) == true)
    {
      for (i=0; i<length; i++)
      {
        i2c_ret = i2cRead16Byte(ch, dev_addr, reg_addr+i, i2c_data, 100);

        if (i2c_ret == true)
        {
          cmdifPrintf("%d I2C - 0x%02X : 0x%02X\n", print_ch, reg_addr+i, i2c_data[0]);
        }
        else
        {
          cmdifPrintf("%d I2C - Fail \n", print_ch);
          break;
        }
      }
    }
    else if(cmdifHasString("write", 0) == true)
    {
      pre_time = micros();
      i2c_ret = i2cWriteByte(ch, dev_addr, reg_addr, (uint8_t)length, 100);

      if (i2c_ret == true)
      {
        cmdifPrintf("%d I2C - 0x%02X : 0x%02X, %d us\n", print_ch, reg_addr, length, micros()-pre_time);
      }
      else
      {
        cmdifPrintf("%d I2C - Fail \n", print_ch);
      }
    }
    else if(cmdifHasString("write2", 0) == true)
    {
      pre_time = micros();
      i2c_ret = i2cWriteByte2(ch, dev_addr, reg_addr, (uint8_t)length, 100);

      if (i2c_ret == true)
      {
        cmdifPrintf("%d I2C - 0x%02X : 0x%02X, %d us\n", print_ch, reg_addr, length, micros()-pre_time);
      }
      else
      {
        cmdifPrintf("%d I2C - Fail \n", print_ch);
      }
    }
    else if(cmdifHasString("write16", 0) == true)
    {
      pre_time = micros();
      i2c_ret = i2cWrite16Byte(ch, dev_addr, reg_addr, (uint8_t)length, 100);

      if (i2c_ret == true)
      {
        cmdifPrintf("%d I2C - 0x%02X : 0x%02X, %d us\n", print_ch, reg_addr, length, micros()-pre_time);
      }
      else
      {
        cmdifPrintf("%d I2C - Fail \n", print_ch);
      }
    }
    else
    {
      ret = false;
    }
  }
  else
  {
    ret = false;
  }

  if (ret == false)
  {
    cmdifPrintf( "i2c begin channel[1~%d]\n", I2C_MAX_CH);
    cmdifPrintf( "i2c scan channel[1~%d]\n", I2C_MAX_CH);
    cmdifPrintf( "i2c read channel dev_addr reg_addr length\n");
    cmdifPrintf( "i2c read2 channel dev_addr reg_addr length\n");
    cmdifPrintf( "i2c read16 channel dev_addr reg_addr length\n");
    cmdifPrintf( "i2c write channel dev_addr reg_addr data\n");
    cmdifPrintf( "i2c write2 channel dev_addr reg_addr data\n");
    cmdifPrintf( "i2c write16 channel dev_addr reg_addr data\n");
  }
}




#endif


#endif
