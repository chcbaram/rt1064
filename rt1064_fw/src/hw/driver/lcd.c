/*
 * lcd.c
 *
 *  Created on: 2020. 1. 31.
 *      Author: Baram
 */




#include "lcd.h"
#include "gpio.h"
#include "hangul/PHan_Lib.h"
#include "fsl_elcdif.h"


#define LCD_BUF_ADDR      0x80000000


#ifndef _swap_int16_t
#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
#endif



static bool is_init = false;
static uint8_t backlight_value = 100;
static uint8_t frame_index = 0;
static volatile bool is_double_buffer = true;
static int32_t text_bg_color = -1;

static uint16_t *p_draw_frame_buf = NULL;
static uint16_t  *frame_buffer[2] =
    {
        (uint16_t *)(LCD_BUF_ADDR),
        (uint16_t *)(LCD_BUF_ADDR + (HW_LCD_WIDTH * HW_LCD_HEIGHT * 2)),
    };

static bool lcd_request_draw = false;

static uint32_t fps_pre_time;
static uint32_t fps_time;
static uint32_t fps_count = 0;

static uint8_t pin_rst = 2;
static uint8_t pin_bkl = 3;


void lcdFillBuffer(void * pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);
void lcdSwapFrameBuffer(void);
void disHanFont(int x, int y, PHAN_FONT_OBJ *FontPtr, uint16_t textcolor);
void lcdHwInit(void);

bool lcdInit(void)
{
  is_init = true;


  backlight_value = 100;


  gpioPinWrite(pin_rst, _DEF_LOW);
  delay(50);
  gpioPinWrite(pin_rst, _DEF_HIGH);

  gpioPinWrite(pin_bkl, _DEF_HIGH);


  for (int i=0; i<HW_LCD_WIDTH*HW_LCD_HEIGHT; i++)
  {
    frame_buffer[0][i] = black;
    frame_buffer[1][i] = black;
  }

  if (is_double_buffer == true)
  {
    p_draw_frame_buf = frame_buffer[frame_index ^ 1];
  }
  else
  {
    p_draw_frame_buf = frame_buffer[frame_index];
  }

  lcdHwInit();

  lcdClear(black);
  lcdSetBackLight(backlight_value);

  lcdDisplayOn();

  lcdDrawFillRect(0, 0, 100, 100, red);
  lcdPrintf(0, 0, white, "테스트 ");
  lcdUpdateDraw();

  return true;
}

void lcdHwInit(void)
{
  const elcdif_rgb_mode_config_t config = {
      .panelWidth    = HW_LCD_WIDTH,
      .panelHeight   = HW_LCD_HEIGHT,
      .hsw           = 41,
      .hfp           = 4,
      .hbp           = 8,
      .vsw           = 10,
      .vfp           = 4,
      .vbp           = 2,
      .polarityFlags = (kELCDIF_DataEnableActiveHigh |
                        kELCDIF_VsyncActiveLow |
                        kELCDIF_HsyncActiveLow |
                        kELCDIF_DriveDataOnRisingClkEdge),
      .bufferAddr    = (uint32_t)frame_buffer[0],
      .pixelFormat   = kELCDIF_PixelFormatRGB565,
      .dataBus       = kELCDIF_DataBus16Bit,
  };

  ELCDIF_RgbModeInit(LCDIF, &config);


  EnableIRQ(LCDIF_IRQn);

  ELCDIF_EnableInterrupts(LCDIF, kELCDIF_CurFrameDoneInterruptEnable);
  ELCDIF_RgbModeStart(LCDIF);
}

bool lcdIsInit(void)
{
  return is_init;
}

void lcdReset(void)
{

}

uint8_t lcdGetBackLight(void)
{
  return backlight_value;
}

void lcdSetBackLight(uint8_t value)
{
  if (value != backlight_value)
  {
    backlight_value = value;
  }
}

uint32_t lcdReadPixel(uint16_t x_pos, uint16_t y_pos)
{
  return p_draw_frame_buf[y_pos * HW_LCD_WIDTH + x_pos];
}

void lcdDrawPixel(uint16_t x_pos, uint16_t y_pos, uint32_t rgb_code)
{
  p_draw_frame_buf[y_pos * HW_LCD_WIDTH + x_pos] = rgb_code;
}

void lcdClear(uint32_t rgb_code)
{
  lcdClearBuffer(rgb_code);

  lcdUpdateDraw();
}

void lcdClearBuffer(uint32_t rgb_code)
{
  uint16_t *p_buf = lcdGetFrameBuffer();

  for (int i=0; i<HW_LCD_WIDTH * HW_LCD_HEIGHT; i++)
  {
    p_buf[i] = rgb_code;
  }
}

void lcdSetDoubleBuffer(bool enable)
{
  is_double_buffer = enable;

  if (enable == true)
  {
    p_draw_frame_buf = frame_buffer[frame_index^1];
  }
  else
  {
    p_draw_frame_buf = frame_buffer[frame_index];
  }
}

bool lcdGetDoubleBuffer(void)
{
  return is_double_buffer;
}

bool lcdDrawAvailable(void)
{
  return !lcd_request_draw;
}

void lcdRequestDraw(void)
{
  if (lcd_request_draw != true)
  {
    ELCDIF_SetNextBufferAddr(LCDIF, (uint32_t)lcdGetFrameBuffer());
    lcd_request_draw = true;
  }
}

void lcdUpdateDraw(void)
{
  lcdRequestDraw();
  while(lcdDrawAvailable() != true);
}

void lcdSetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{

}

void lcdSwapFrameBuffer(void)
{
  if (lcd_request_draw == true)
  {
    frame_index ^= 1;

    if (is_double_buffer == true)
    {
      p_draw_frame_buf = frame_buffer[frame_index ^ 1];
    }
    else
    {
      p_draw_frame_buf = frame_buffer[frame_index];
    }

    lcd_request_draw = false;
  }
}

uint16_t *lcdGetFrameBuffer(void)
{
  return p_draw_frame_buf;
}

uint16_t *lcdGetCurrentFrameBuffer(void)
{
  return frame_buffer[frame_index];;
}

void lcdDisplayOff(void)
{
  gpioPinWrite(pin_bkl, _DEF_LOW);
}

void lcdDisplayOn(void)
{
  gpioPinWrite(pin_bkl, _DEF_HIGH);
}

int32_t lcdGetWidth(void)
{
  return HW_LCD_WIDTH;
}

int32_t lcdGetHeight(void)
{
  return HW_LCD_HEIGHT;
}


void lcdDrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color)
{
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);

  if (steep)
  {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1)
  {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1)
  {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++)
  {
    if (steep)
    {
      lcdDrawPixel(y0, x0, color);
    } else
    {
      lcdDrawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0)
    {
      y0 += ystep;
      err += dx;
    }
  }
}

void lcdDrawVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
  lcdDrawLine(x, y, x, y+h-1, color);
}

void lcdDrawHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  lcdDrawLine(x, y, x+w-1, y, color);
}

void lcdDrawFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  for (int16_t i=x; i<x+w; i++)
  {
    lcdDrawVLine(i, y, h, color);
  }
}

void lcdDrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  lcdDrawHLine(x, y, w, color);
  lcdDrawHLine(x, y+h-1, w, color);
  lcdDrawVLine(x, y, h, color);
  lcdDrawVLine(x+w-1, y, h, color);
}

void lcdDrawFillScreen(uint16_t color)
{
  lcdDrawFillRect(0, 0, HW_LCD_WIDTH, HW_LCD_HEIGHT, color);
}

void lcdSetBgColor(int32_t bg_color)
{
  text_bg_color = bg_color;
}

int32_t lcdGetFpsTime(void)
{
  return fps_time;
};

int32_t lcdGetFps(void)
{
  return fps_count;
}

void lcdPrintf(int x, int y, uint16_t color,  const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i, x_Pre = x;
  PHAN_FONT_OBJ FontBuf;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  for( i=0; i<len; i+=Size_Char )
  {
    PHan_FontLoad( &print_buffer[i], &FontBuf );


    disHanFont( x, y, &FontBuf, color);

    Size_Char = FontBuf.Size_Char;
    if (Size_Char >= 2)
    {
        x += 2*8;
    }
    else
    {
        x += 1*8;
    }

    if( HW_LCD_WIDTH < x )
    {
        x  = x_Pre;
        y += 16;
    }

    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }
}

uint32_t lcdGetStrWidth(const char *fmt, ...)
{
  va_list arg;
  va_start (arg, fmt);
  int32_t len;
  char print_buffer[256];
  int Size_Char;
  int i;
  PHAN_FONT_OBJ FontBuf;
  uint32_t str_len;


  len = vsnprintf(print_buffer, 255, fmt, arg);
  va_end (arg);

  str_len = 0;

  for( i=0; i<len; i+=Size_Char )
  {
    PHan_FontLoad( &print_buffer[i], &FontBuf );

    Size_Char = FontBuf.Size_Char;

    str_len += (Size_Char * 8);

    if( FontBuf.Code_Type == PHAN_END_CODE ) break;
  }

  return str_len;
}

void disHanFont(int x, int y, PHAN_FONT_OBJ *FontPtr, uint16_t textcolor)
{
  uint16_t    i, j, Loop;
  uint16_t  FontSize = FontPtr->Size_Char;
  uint16_t index_x;

  if (FontSize > 2)
  {
    FontSize = 2;
  }

  for ( i = 0 ; i < 16 ; i++ )        // 16 Lines per Font/Char
  {
    index_x = 0;
    for ( j = 0 ; j < FontSize ; j++ )      // 16 x 16 (2 Bytes)
    {
      for( Loop=0; Loop<8; Loop++ )
      {
        if( FontPtr->FontBuffer[i*FontSize +j] & (0x80>>Loop))
        {
          lcdDrawPixel(x + index_x, y + i, textcolor);
        }
        else if (text_bg_color >= 0)
        {
          lcdDrawPixel(x + index_x, y + i, text_bg_color & 0xFFFF);
        }

        index_x++;
      }
    }
  }
}





void LCDIF_IRQHandler(void)
{
  uint32_t intStatus;

  intStatus = ELCDIF_GetInterruptStatus(LCDIF);

  ELCDIF_ClearInterruptStatus(LCDIF, intStatus);

  if (intStatus & kELCDIF_CurFrameDone)
  {
    lcdSwapFrameBuffer();

    fps_time = millis()-fps_pre_time;
    if (fps_time > 0)
    {
      fps_count = 1000 / fps_time;
    }
    fps_pre_time = millis();
  }
}
