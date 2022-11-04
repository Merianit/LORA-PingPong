#include "st7735.h"
#include "stdlib.h"
#include "string.h"

static volatile _Bool dispSpiAvailable = true; // 0 если SPI занят, 1 - если нет

int16_t _height, _width;

// Двойная буффериазация 
static uint8_t dispBuffer1[SIZEBUF];
static uint8_t dispBuffer2[SIZEBUF];
static uint8_t *dispBuffer=dispBuffer1;

// Константы для задержек
#define DELAY 									0x80
#define ST7735_RESET_DELAY			5
#define ST7735_FLAG_LONG_DELAY	255
#define ST7735_LONG_DELAY				500

static uint8_t _data_rotation[4] = { ST7735_MADCTL_MX, ST7735_MADCTL_MY, ST7735_MADCTL_MV, ST7735_MADCTL_RGB };
static ST7735_Orientat_t _value_rotation = Displ_Orientat_90;

static uint8_t _xstart = ST7735_XSTART, _ystart = ST7735_YSTART;

static const uint8_t
init_cmds1[] = {            		// Init for 7735R, part 1 (red or green tab)
		  15,                       // 15 commands in list:
		  ST7735_SWRESET, DELAY,  	//  1: Software reset, 0 args, w/delay
		  150,                    	//     150 ms delay
		  ST7735_SLPOUT, DELAY,  	//  2: Out of sleep mode, 0 args, w/delay
		  ST7735_FLAG_LONG_DELAY,   //     500 ms delay
		  ST7735_FRMCTR1, 3,		//  3: Frame rate ctrl - normal mode, 3 args:
		  0x01, 0x2C, 0x2D,       	//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		  ST7735_FRMCTR2, 3,  		//  4: Frame rate control - idle mode, 3 args:
		  0x01, 0x2C, 0x2D,       	//     Rate = fosc/(1x2+40) * (LINE+2C+2D)
		  ST7735_FRMCTR3, 6,  		//  5: Frame rate ctrl - partial mode, 6 args:
		  0x01, 0x2C, 0x2D,       	//     Dot inversion mode
		  0x01, 0x2C, 0x2D,       	//     Line inversion mode
		  ST7735_INVCTR, 1,  		//  6: Display inversion ctrl, 1 arg, no delay:
		  0x07,                   	//     No inversion
		  ST7735_PWCTR1, 3,  		//  7: Power control, 3 args, no delay:
		  0xA2,
		  0x02,                   	//     -4.6V
		  0x84,                   	//     AUTO mode
		  ST7735_PWCTR2, 1,  		//  8: Power control, 1 arg, no delay:
		  0xC5,                   	//     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
		  ST7735_PWCTR3, 2,  		//  9: Power control, 2 args, no delay:
		  0x0A,                   	//     Opamp current small
		  0x00,                   	//     Boost frequency
		  ST7735_PWCTR4, 2,  		// 10: Power control, 2 args, no delay:
		  0x8A,                   	//     BCLK/2, Opamp current small & Medium low
		  0x2A,
		  ST7735_PWCTR5, 2,  		// 11: Power control, 2 args, no delay:
		  0x8A, 0xEE,
		  ST7735_VMCTR1, 1,  		// 12: Power control, 1 arg, no delay:
		  0x0E,
		  ST7735_INVOFF, 0,  		// 13: Don't invert display, no args, no delay
		  ST7735_MADCTL, 1,  		// 14: Memory access control (directions), 1 arg:
		  ST7735_ROTATION,     //     row addr/col addr, bottom to top refresh
		  ST7735_COLMOD, 1,  		// 15: set color mode, 1 arg, no delay:
		  0x05},                 	//     16-bit color


init_cmds2[] = {            // Init for 7735R, part 2 (1.44" display)
		2,                  //  2 commands in list:
		ST7735_CASET, 4,  	//  1: Column addr set, 4 args, no delay:
		0x00, 0x00,         //     XSTART = 0
		0x00, 0x7F,         //     XEND = 127
		ST7735_RASET, 4,  	//  2: Row addr set, 4 args, no delay:
		0x00, 0x00,         //     XSTART = 0
		0x00, 0x7F },       //     XEND = 127

init_cmds3[] = {            		// Init for 7735R, part 3 (red or green tab)
		4,                        	//  4 commands in list:
		ST7735_GMCTRP1, 16, 		//  1: Magical unicorn dust, 16 args, no delay:
		0x02, 0x1c, 0x07, 0x12,
		0x37, 0x32, 0x29, 0x2d,
		0x29, 0x25, 0x2B, 0x39,
		0x00, 0x01, 0x03, 0x10,
		ST7735_GMCTRN1, 16, 		//  2: Sparkles and rainbows, 16 args, no delay:
		0x03, 0x1d, 0x07, 0x06,
		0x2E, 0x2C, 0x29, 0x2D,
		0x2E, 0x2E, 0x37, 0x3F,
		0x00, 0x00, 0x02, 0x10,
		ST7735_NORON, DELAY, 		//  3: Normal display on, no args, w/delay
		10,                     	//     10 ms delay
		ST7735_DISPON, DELAY, 		//  4: Main screen turn on, no args w/delay
		100 };                  	//     100 ms delay



static void ST7735_Reset();
static void ST7735_Select();
static void ST7735_Unselect();

static void ST7735_Transmit(GPIO_PinState DC_Status, uint8_t* data, uint16_t dataSize );		
static void ST7735_WriteCommand(uint8_t cmd);
static void ST7735_WriteData(uint8_t* buff, size_t buff_size);
static void ST7735_ExecuteCommandList(const uint8_t *addr);
static void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
		
static void ST7735_Reset()
{
	HAL_GPIO_WritePin(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_RESET);
	HAL_Delay(ST7735_RESET_DELAY);
	HAL_GPIO_WritePin(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_SET);
}

static void ST7735_Select() {
	HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
}

static void ST7735_Unselect() {
	HAL_GPIO_WritePin(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_SET);
}

static void ST7735_WriteCommand(uint8_t cmd){
	ST7735_Transmit(SPI_COMMAND, &cmd, sizeof(cmd));
}

static void ST7735_WriteData(uint8_t* buff, size_t buff_size){
	if (buff_size==0) return;
	ST7735_Transmit(SPI_DATA, buff, buff_size);
}

static void ST7735_Transmit(GPIO_PinState DC_Status, uint8_t* data, uint16_t dataSize ) {
	while (!dispSpiAvailable) {};  // ожидаем пока SPI будет свободен
	ST7735_Select();
	HAL_GPIO_WritePin(ST7735_DC_GPIO_Port, ST7735_DC_Pin, DC_Status);

	#ifndef DISPLAY_SPI_POLLING_MODE
		if (dataSize<DISPL_DMA_CUTOFF) {
			dispSpiAvailable = 0;
			HAL_SPI_Transmit(&ST7735_SPI_PORT, data, dataSize, HAL_MAX_DELAY);
			dispSpiAvailable = 1;
			ST7735_Unselect();
		}
		else {
			dispSpiAvailable=0;
			#ifdef DISPLAY_SPI_DMA_MODE
				HAL_SPI_Transmit_DMA(&ST7735_SPI_PORT , data, dataSize);
			#endif
			#ifdef DISPLAY_SPI_INTERRUPT_MODE
				HAL_SPI_Transmit_IT(&ST7735_SPI_PORT , data, dataSize);
			#endif
		}
	#endif

	#ifdef DISPLAY_SPI_POLLING_MODE
		dispSpiAvailable = 0;
		HAL_SPI_Transmit(&ST7735_SPI_PORT , data, dataSize, HAL_MAX_DELAY);
		dispSpiAvailable = 1;
		ST7735_Unselect();
	#endif
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	__NOP();
	if (hspi->Instance == ST7735_SPI) {
		dispSpiAvailable = 1;
		ST7735_Unselect();
	}
}




void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi->Instance == ST7735_SPI) {
		dispSpiAvailable = 1;
		ST7735_Unselect();
	}
}

static void ST7735_ExecuteCommandList(const uint8_t *addr)
{
    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;
    while(numCommands--)
    {
    	uint8_t cmd = *addr++;
        ST7735_WriteCommand(cmd);

        numArgs = *addr++;
        // If high bit set, delay follows args
        ms = numArgs & DELAY;
        numArgs &= ~DELAY;
        if(numArgs)
        {
            ST7735_WriteData((uint8_t*)addr, numArgs);
            addr += numArgs;
        }

        if(ms)
        {
            ms = *addr++;
            if(ms == ST7735_FLAG_LONG_DELAY) ms = ST7735_LONG_DELAY;
            HAL_Delay(ms);
        }
    }
}

static void ST7735_SetAddressWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    // column address set
    ST7735_WriteCommand(ST7735_CASET);
    uint8_t data[] = { 0x00, x0 + _xstart, 0x00, x1 + _xstart };
    ST7735_WriteData(data, sizeof(data));

    // row address set
    ST7735_WriteCommand(ST7735_RASET);
    data[1] = y0 + _ystart;
    data[3] = y1 + _ystart;
    ST7735_WriteData(data, sizeof(data));

    // write to RAM
    ST7735_WriteCommand(ST7735_RAMWR);
}

void ST7735_Init(ST7735_Orientat_t m)
{
	ST7735_Select();
	ST7735_Reset();
	ST7735_ExecuteCommandList(init_cmds1);
	ST7735_ExecuteCommandList(init_cmds2);
	ST7735_ExecuteCommandList(init_cmds3);
	ST7735_Unselect();
	ST7735_SetRotation(m);
}

void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if((x >= _width) || (y >= _height))
        return;
		ST7735_FillRectangle(x, y, 1, 1, color);
}

void ST7735_DrawWChar(uint16_t x, uint16_t y, char ch, sFONT font, uint8_t size, uint16_t color, uint16_t bgcolor) {
	uint32_t i, b, bytes, j, bufSize, mask;
	uint16_t *dispBuffer16=(uint16_t *)dispBuffer;

	const uint8_t *pos;
	uint8_t wsize=font.Width; //printing char width
	uint16_t color1, bgcolor1;

	if (size==2)
	wsize<<= 1;
	bufSize=0;
	bytes=font.Height * font.Size ;
	pos=font.table+(ch - 32) * bytes ;//that's char position in table
	switch (font.Size) {
		case 3:
			mask=0x800000;
			break;
		case 2:
			mask=0x8000;
			break;
		default:
			mask=0x80;
	}

	color1 = ((color & 0xFF)<<8 | (color >> 8));      		//swapping byte endian: STM32 is little endian, ST7735 is big endian
	bgcolor1 = ((bgcolor & 0xFF)<<8 | (bgcolor >> 8));		//swapping byte endian: STM32 is little endian, ST7735 is big endian

	for(i = 0; i < (bytes); i+=font.Size){
		b=0;
		switch (font.Size) {
			case 3:
				b=pos[i]<<16 | pos[i+1]<<8 | pos[i+2];
				break;
			case 2:
				b=pos[i]<<8 | pos[i+1];
				break;
			default:
				b=pos[i];
		}

		for(j = 0; j < font.Width; j++) {
			if((b << j) & mask)  {
				dispBuffer16[bufSize++] = color1;
				if (size==2){
					dispBuffer16[bufSize++] = color1;
				}
			} else {
				dispBuffer16[bufSize++] = bgcolor1;
				if (size==2) {
					dispBuffer16[bufSize++] = bgcolor1;
				}
			}
		}
	}
	ST7735_SetAddressWindow(x, y, x+wsize-1, y+font.Height-1);
	ST7735_WriteData(dispBuffer,bufSize<<1);
	dispBuffer = (dispBuffer==dispBuffer1 ? dispBuffer2 : dispBuffer1); // swapping buffer
}

void ST7735_DrawWString(uint16_t x, uint16_t y, const char* str, sFONT font, uint8_t size, uint16_t color, uint16_t bgcolor) {
	uint16_t delta=font.Width;
	if (size>1)
		delta<<=1;

		while(*str) {
				ST7735_DrawWChar(x, y, *str, font, size, color, bgcolor);
				x += delta;
				str++;
		}
}

void ST7735_DrawCSString(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, const char* str, sFONT font, uint8_t size, uint16_t color, uint16_t bgcolor) {
	uint16_t x,y;
	uint16_t wsize=font.Width;
	static uint8_t cambia=0;
	if (size>1)
		wsize<<=1;
	if ((strlen(str)*wsize)>(x1-x0+1))
		x=x0;
	else
		x=(x1+x0+1-strlen(str)*wsize) >> 1;
	if (font.Height>(y1-y0+1))
		y=y0;
	else
		y=(y1+y0+1-font.Height) >> 1;

	if (x>x0){
		ST7735_FillRectangle(x0,y0,x-x0,y1-y0+1,bgcolor);
	} else
		x=x0; // fixing here mistake could be due to roundings: x lower than x0.
	if (x1>(strlen(str)*wsize+x0))
		ST7735_FillRectangle(x1-x+x0-1,y0,x-x0+1,y1-y0+1,bgcolor);

	if (y>y0){
		ST7735_FillRectangle(x0,y0,x1-x0+1,y-y0,bgcolor);
	} else
		y=y0; //same comment as above
	if (y1>=(font.Height+y0))
		ST7735_FillRectangle(x0,y1-y+y0,x1-x0+1,y-y0+1,bgcolor);

	cambia = !cambia;

	ST7735_DrawWString(x, y, str, font, size, color, bgcolor);
}

void ST7735_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	uint16_t k,x1,y1,area,times;

	uint32_t data32;

	if((x >= _width) || (y >= _height) || (w == 0) || (h == 0)) return;//

	x1 = x + w - 1;
	if (x1 > _width) {
		x1 = _width;
	}

	y1 = y + h - 1;
	if (y1 > _height) {
		y1 = _height;
	}


	data32 = (color>>8) | (color<<8) | (color<<24); 	// supposing color is 0xABCD, data32 becomes 0xCDABCDAB - set a 32 bit variable with swapped endians
	area=((y1-y+1)*(x1-x+1)); 						// area to fill in 16bit pixels
	uint32_t *buf32Pos=(uint32_t *)dispBuffer; 		//dispBuffer defined in bytes, buf32Pos access it as 32 bit words
	if (area < (SIZEBUF >> 1)) 							// if area is smaller than dispBuffer
		times=(area >> 1) + 1; 							// number of times data32 has to be loaded into buffer
	else
		times=(SIZEBUF >> 2);  						// dispBuffer size as 32bit-words
	for (k = 0; k < times; k++)
		*(buf32Pos++) = data32; 						// loads buffer moving 32bit-words
	times=(area >> (BUFLEVEL-1));  					//how many times buffer must be sent via SPI. It is (BUFFLEVEL-1) because area is 16-bit while dispBuffer is 8-bit

	ST7735_SetAddressWindow(x, y, x1, y1);

	for  (k=0;k<times;k++) {
		ST7735_WriteData(dispBuffer,SIZEBUF);
	}
	ST7735_WriteData(dispBuffer,(area<<1)-(times<<BUFLEVEL));
	dispBuffer = (dispBuffer==dispBuffer1 ? dispBuffer2 : dispBuffer1); // swapping buffer
}

void ST7735_FillScreen(uint16_t color)
{
    ST7735_FillRectangle(0, 0, _width, _height, color);
}

void ST7735_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data)
{
  if((x >= _width) || (y >= _height)) return;
  if((x + w - 1) >= _width) return;
  if((y + h - 1) >= _height) return;

  ST7735_SetAddressWindow(x, y, x+w-1, y+h-1);
  ST7735_WriteData((uint8_t*)data, sizeof(uint16_t)*w*h);
}

void ST7735_InvertColors(bool invert)
{
	ST7735_WriteCommand(invert ? ST7735_INVON : ST7735_INVOFF);
}

void ST7735_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	//    writePixel(x0  , y0+r, color);
	ST7735_DrawPixel(x0  , y0+r, color);
	ST7735_DrawPixel(x0  , y0-r, color);
	ST7735_DrawPixel(x0+r, y0  , color);
	ST7735_DrawPixel(x0-r, y0  , color);

	while (x<y) {
			if (f >= 0) {
					y--;
					ddF_y += 2;
					f += ddF_y;
			}
			x++;
			ddF_x += 2;
			f += ddF_x;

			ST7735_DrawPixel(x0 + x, y0 + y, color);
			ST7735_DrawPixel(x0 - x, y0 + y, color);
			ST7735_DrawPixel(x0 + x, y0 - y, color);
			ST7735_DrawPixel(x0 - x, y0 - y, color);
			ST7735_DrawPixel(x0 + y, y0 + x, color);
			ST7735_DrawPixel(x0 - y, y0 + x, color);
			ST7735_DrawPixel(x0 + y, y0 - x, color);
			ST7735_DrawPixel(x0 - y, y0 - x, color);
	}
}


void ST7735_DrawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color)
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;

	while (x<y) {
			if (f >= 0) {
					y--;
					ddF_y += 2;
					f     += ddF_y;
			}
			x++;
			ddF_x += 2;
			f     += ddF_x;
			if (cornername & 0x4) {
					ST7735_DrawPixel(x0 + x, y0 + y, color);
					ST7735_DrawPixel(x0 + y, y0 + x, color);
			}
			if (cornername & 0x2) {
				ST7735_DrawPixel(x0 + x, y0 - y, color);
				ST7735_DrawPixel(x0 + y, y0 - x, color);
			}
			if (cornername & 0x8) {
				ST7735_DrawPixel(x0 - y, y0 + x, color);
				ST7735_DrawPixel(x0 - x, y0 + y, color);
			}
			if (cornername & 0x1) {
				ST7735_DrawPixel(x0 - y, y0 - x, color);
				ST7735_DrawPixel(x0 - x, y0 - y, color);
			}
	}
}


void ST7735_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
	ST7735_DrawLine(x0, y0-r, x0, y0+r, color);
	ST7735_FillCircleHelper(x0, y0, r, 3, 0, color);
}

void ST7735_FillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color)
{
	int16_t f     = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x     = 0;
	int16_t y     = r;
	int16_t px    = x;
	int16_t py    = y;

	delta++; // Avoid some +1's in the loop

	while(x < y) {
			if (f >= 0) {
					y--;
					ddF_y += 2;
					f     += ddF_y;
			}
			x++;
			ddF_x += 2;
			f     += ddF_x;
			// These checks avoid double-drawing certain lines, important
			// for the SSD1306 library which has an INVERT drawing mode.
			if(x < (y + 1)) {
	//            if(corners & 1) drawFastVLine(x0+x, y0-y, 2*y+delta, color);
					if(cornername & 1) ST7735_DrawLine(x0+x, y0-y, x0+x, y0-1+y+delta, color);
	//            if(corners & 2) drawFastVLine(x0-x, y0-y, 2*y+delta, color);
					if(cornername & 2) ST7735_DrawLine(x0-x, y0-y, x0-x, y0-1+y+delta, color);
			}
			if(y != py) {
	//            if(corners & 1) drawFastVLine(x0+py, y0-px, 2*px+delta, color);
					if(cornername & 1) ST7735_DrawLine(x0+py, y0-px, x0+py, y0-1+px+delta, color);
	//            if(corners & 2) drawFastVLine(x0-py, y0-px, 2*px+delta, color);
					if(cornername & 1) ST7735_DrawLine(x0-py, y0-px, x0-py, y0-1+px+delta, color);
					py = y;
			}
			px = x;
	}
}


void ST7735_DrawEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color)
{
  if (rx < 2) return;
  if (ry < 2) return;
  int16_t x, y;
  int32_t rx2 = rx * rx;
  int32_t ry2 = ry * ry;
  int32_t fx2 = 4 * rx2;
  int32_t fy2 = 4 * ry2;
  int32_t s;

  for (x = 0, y = ry, s = 2 * ry2 + rx2 * (1-2 * ry); ry2 * x <= rx2 * y; x++)
  {
	  ST7735_DrawPixel(x0 + x, y0 + y, color);
	  ST7735_DrawPixel(x0 - x, y0 + y, color);
	  ST7735_DrawPixel(x0 - x, y0 - y, color);
	  ST7735_DrawPixel(x0 + x, y0 - y, color);
    if (s >= 0)
    {
      s += fx2 * (1 - y);
      y--;
    }
    s += ry2 * ((4 * x) + 6);
  }

  for (x = rx, y = 0, s = 2 * rx2 + ry2 * (1 - 2 * rx); rx2 * y <= ry2 * x; y++)
  {
	  ST7735_DrawPixel(x0 + x, y0 + y, color);
	  ST7735_DrawPixel(x0 - x, y0 + y, color);
	  ST7735_DrawPixel(x0 - x, y0 - y, color);
	  ST7735_DrawPixel(x0 + x, y0 - y, color);
	if (s >= 0)
	{
	  s += fy2 * (1 - x);
	  x--;
	}
	s += rx2 * ((4 * y) + 6);
  }
}


void ST7735_FillEllipse(int16_t x0, int16_t y0, int16_t rx, int16_t ry, uint16_t color)
{
  if (rx < 2) return;
  if (ry < 2) return;
  int16_t x, y;
  int32_t rx2 = rx * rx;
  int32_t ry2 = ry * ry;
  int32_t fx2 = 4 * rx2;
  int32_t fy2 = 4 * ry2;
  int32_t s;

  for (x = 0, y = ry, s = 2 * ry2 + rx2 * (1 - 2 * ry); ry2 * x <= rx2 * y; x++)
  {
    ST7735_DrawLine(x0 - x, y0 - y, x + x + 1, y0 - y, color);
    ST7735_DrawLine(x0 - x, y0 + y, x + x + 1, y0 + y, color);

    if (s >= 0)
    {
      s += fx2 * (1 - y);
      y--;
    }
    s += ry2 * ((4 * x) + 6);
  }

  for (x = rx, y = 0, s = 2 * rx2 + ry2 * (1 - 2 * rx); rx2 * y <= ry2 * x; y++)
  {
    ST7735_DrawLine(x0 - x, y0 - y, x + x + 1, y0 - y, color);
    ST7735_DrawLine(x0 - x, y0 + y, x + x + 1, y0 + y, color);

    if (s >= 0)
    {
      s += fy2 * (1 - x);
      x--;
    }
    s += rx2 * ((4 * y) + 6);
  }

}

// Draw a triangle
void ST7735_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  ST7735_DrawLine(x0, y0, x1, y1, color);
  ST7735_DrawLine(x1, y1, x2, y2, color);
  ST7735_DrawLine(x2, y2, x0, y0, color);
}


// Fill a triangle - original Adafruit function works well and code footprint is small
void ST7735_FillTriangle( int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
			_swap_int16_t(&y0, &y1); _swap_int16_t(&x0, &x1);
	}
	if (y1 > y2) {
			_swap_int16_t(&y2, &y1); _swap_int16_t(&x2, &x1);
	}
	if (y0 > y1) {
			_swap_int16_t(&y0, &y1); _swap_int16_t(&x0, &x1);
	}

	if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
			a = b = x0;
			if(x1 < a)      a = x1;
			else if(x1 > b) b = x1;
			if(x2 < a)      a = x2;
			else if(x2 > b) b = x2;
	//        drawFastHLine(a, y0, b-a+1, color);
			ST7735_DrawLine(a, y0, b, y0, color);
			return;
	}

	int16_t
	dx01 = x1 - x0,
	dy01 = y1 - y0,
	dx02 = x2 - x0,
	dy02 = y2 - y0,
	dx12 = x2 - x1,
	dy12 = y2 - y1;
	int32_t
	sa   = 0,
	sb   = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if(y1 == y2) last = y1;   // Include y1 scanline
	else         last = y1-1; // Skip it

	for(y=y0; y<=last; y++) {
			a   = x0 + sa / dy01;
			b   = x0 + sb / dy02;
			sa += dx01;
			sb += dx02;
			/* longhand:
			a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
			b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
			*/
			if(a > b) _swap_int16_t(&a,&b);
	//        drawFastHLine(a, y, b-a+1, color);
			ST7735_DrawLine(a, y, b, y, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = (int32_t)dx12 * (y - y1);
	sb = (int32_t)dx02 * (y - y0);
	for(; y<=y2; y++) {
			a   = x1 + sa / dy12;
			b   = x0 + sb / dy02;
			sa += dx12;
			sb += dx02;
			/* longhand:
			a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
			b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
			*/
			if(a > b) _swap_int16_t(&a,&b);
	//      drawFastHLine(a, y, b-a+1, color);
			ST7735_DrawLine(a, y, b, y, color);
	}
}

void ST7735_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if(r > max_radius) r = max_radius;
    // smarter version
//    drawFastHLine(x+r  , y    , w-2*r, color); // Top
    ST7735_DrawLine(x+r, y, x+w-r-1, y, color);
//    drawFastHLine(x+r, y+h-1, w-2*r, color); // Bottom
    ST7735_DrawLine(x+r, y+h-1, x-1+w-r, y+h-1, color);
//    drawFastVLine(x, y+r, h-2*r, color); // Left
    ST7735_DrawLine(x, y+r, x, y-1+h-r, color); // Left
//    drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
    ST7735_DrawLine(x+w-1, y+r, x+w-1, y-1+h-r, color); // Right
    // draw four corners
    ST7735_DrawCircleHelper(x+r    , y+r    , r, 1, color);
    ST7735_DrawCircleHelper(x+w-r-1, y+r    , r, 2, color);
    ST7735_DrawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
    ST7735_DrawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

void ST7735_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color)
{
    int16_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
    if(r > max_radius) r = max_radius;
    // smarter version
    ST7735_FillRectangle(x+r, y, w-2*r, h, color);
    // draw four corners
    ST7735_FillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
    ST7735_FillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

void ST7735_DrawBorder(int16_t x, int16_t y, uint16_t w, uint16_t h, int16_t t, uint16_t color) {
	ST7735_FillRectangle(x, y, w, t, color);
	ST7735_FillRectangle(x, y+h-t, w, t, color);
	ST7735_FillRectangle(x, y, t, h, color);
	ST7735_FillRectangle(x+w-t, y, t, h, color);
}

void ST7735_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	int16_t l,x,steep,ystep,err,dx, dy;

    if (x0==x1){  // fast solve vertical lines
    	if (y1>y0){
    		ST7735_FillRectangle(x0, y0, 1, y1-y0+1, color);
    	}
    	else {
    		ST7735_FillRectangle(x0, y1, 1, y0-y1+1, color);
    	}
    	return;
    }
    if (y0==y1){ // fast solve horizontal lines
    	if (x1>x0)
    		ST7735_FillRectangle(x0, y0, x1-x0+1, 1, color);
    	else
    		ST7735_FillRectangle(x1, y1, x0-x1+1, 1, color);
    	return;
    }

    steep = (y1>y0 ? y1-y0 : y0-y1) > (x1>x0 ? x1-x0 : x0-x1);

    if (steep) {
        _swap_int16_t(&x0, &y0);
        _swap_int16_t(&x1, &y1);
    }

    if (x0 > x1) {
        _swap_int16_t(&x0, &x1);
        _swap_int16_t(&y0, &y1);
    }

    dx = x1 - x0;
    err = dx >> 1;
    if (y0 < y1) {
        dy = y1-y0;
        ystep =  1 ;
    } else {
        dy = y0-y1;
        ystep =  -1 ;
    }

    l=00;
    for (x=x0; x<=x1; x++) {
    	l++;
        err -= dy;
        if (err < 0) {
        	if (steep) {
        		ST7735_FillRectangle(y0, x0, 1, l, color);
            } else {
            	ST7735_FillRectangle(x0, y0, l, 1, color);
            }
            y0 += ystep;
            l=0;
            x0=x+1;
            err += dx;
        }
    }
    if (l!=0){
    	if (steep) {
    		ST7735_FillRectangle(y0, x0, 1, l-1, color);
    	} else {
    		ST7735_FillRectangle(x0, y0, l-1,1, color);
    	}
    }
}

void ST7735_SetRotation(ST7735_Orientat_t m)
{
  ST7735_WriteCommand(ST7735_MADCTL);
	
	_value_rotation = m;
	
  switch (_value_rotation)
  {
    case Displ_Orientat_0:
    {
    	uint8_t d_r = (_data_rotation[0] | _data_rotation[1] | _data_rotation[3]);
    	ST7735_WriteData(&d_r, sizeof(d_r));
        _width  = ST7735_WIDTH;
        _height = ST7735_HEIGHT;
        _xstart = ST7735_XSTART;
		_ystart = ST7735_YSTART;
    }
     break;
    case Displ_Orientat_90:
    {
    	uint8_t d_r = (_data_rotation[1] | _data_rotation[2] | _data_rotation[3]);
    	ST7735_WriteData(&d_r, sizeof(d_r));
    	_width  = ST7735_HEIGHT;
    	_height = ST7735_WIDTH;
    	_xstart = ST7735_YSTART;
    	_ystart = ST7735_XSTART;
    }
      break;
    case Displ_Orientat_180:
    {
    	uint8_t d_r = _data_rotation[3];
    	ST7735_WriteData(&d_r, sizeof(d_r));
    	_width  = ST7735_WIDTH;
    	_height = ST7735_HEIGHT;
    	_xstart = ST7735_XSTART;
    	_ystart = ST7735_YSTART;
    }
      break;
    case Displ_Orientat_270:
    {
    	uint8_t d_r = (_data_rotation[0] | _data_rotation[2] | _data_rotation[3]);
    	ST7735_WriteData(&d_r, sizeof(d_r));
    	_width  = ST7735_HEIGHT;
    	_height = ST7735_WIDTH;
    	_xstart = ST7735_YSTART;
    	_ystart = ST7735_XSTART;
    }
      break;
  }
}

ST7735_Orientat_t ST7735_GetRotation(void)
{
  return _value_rotation;
}

uint32_t ST7735_SetBacklight(uint8_t cmd) {

	#ifdef DISPLAY_DIMMING_MODE
	static uint16_t memCCR1 = 0;  			//it stores CCR1 value while in stand-by
	#endif

	switch (cmd) {
	#ifndef DISPLAY_DIMMING_MODE
	case 'F':
	case '1':
		HAL_GPIO_WritePin(ST7735_BL_GPIO_Port, ST7735_BL_Pin, GPIO_PIN_SET);
		break;
	case '0':
		HAL_GPIO_WritePin(ST7735_BL_GPIO_Port, ST7735_BL_Pin, GPIO_PIN_RESET);
		break;
	#else
	case 'F':
	case '1':
		ST7735_TIMER->ST7735_TIMER_CCR=ST7735_TIMER->ARR;
		break;
	case '0':
		ST7735_TIMER->ST7735_TIMER_CCR=0;
		break;
	case 'W':
		ST7735_TIMER->ST7735_TIMER_CCR=memCCR1;					//restore previous level
		break;
	case 'S':
		memCCR1=ST7735_TIMER->ST7735_TIMER_CCR;
		if (ST7735_TIMER->ST7735_TIMER_CCR>=(ST7735_STBY_LEVEL))	//set stby level only if current level is higher
			ST7735_TIMER->ST7735_TIMER_CCR=(ST7735_STBY_LEVEL);
		break;
	case '+':
		if (ST7735_TIMER->ARR>ST7735_TIMER->ST7735_TIMER_CCR)		// if CCR1 has not yet the highest value (ARR)
			++ST7735_TIMER->ST7735_TIMER_CCR;
		else
			ST7735_TIMER->ST7735_TIMER_CCR=ST7735_TIMER->ARR;
		break;
	case '-':
		if (ST7735_TIMER->ST7735_TIMER_CCR>0)					// if CCR1 has not yet the lowest value (0)
			--ST7735_TIMER->ST7735_TIMER_CCR;
		else
			ST7735_TIMER->ST7735_TIMER_CCR=0;
		break;
	case 'I':
		HAL_TIM_PWM_Start(&ST7735_T, ST7735_TIMER_CHANNEL);
		ST7735_TIMER->ST7735_TIMER_CCR = ST7735_INIT_LEVEL;
		break;
	#endif
	case 'Q':
		break;
	default:
		break;
	}
	#ifndef DISPLAY_DIMMING_MODE
	return HAL_GPIO_ReadPin(ST7735_BL_GPIO_Port, ST7735_BL_Pin);
	#else
	return (ST7735_TIMER->ST7735_TIMER_CCR);
	#endif
}
