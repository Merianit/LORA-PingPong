#include "st7735_test.h"

extern int16_t _width;       								///< (oriented) display width
extern int16_t _height;      								///< (oriented) display height


void testLines(uint16_t color)
{
    int           x1, y1, x2, y2,
                  w = _width,
                  h = _height;

    ST7735_FillScreen(ST7735_BLACK);

    x1 = y1 = 0;
    y2    = h - 1;
//    for (x2 = 0; x2 < w; x2 += 6) drawLine(x1, y1, x2, y2, color);
    for (x2 = 0; x2 < w; x2 += 6) ST7735_DrawLine(x1, y1, x2, y2, color);
    x2    = w - 1;
    for (y2 = 0; y2 < h; y2 += 6) ST7735_DrawLine(x1, y1, x2, y2, color);

    ST7735_FillScreen(ST7735_BLACK);

    x1    = w - 1;
    y1    = 0;
    y2    = h - 1;
    for (x2 = 0; x2 < w; x2 += 6) ST7735_DrawLine(x1, y1, x2, y2, color);
    x2    = 0;
    for (y2 = 0; y2 < h; y2 += 6) ST7735_DrawLine(x1, y1, x2, y2, color);

    ST7735_FillScreen(ST7735_BLACK);

    x1    = 0;
    y1    = h - 1;
    y2    = 0;
    for (x2 = 0; x2 < w; x2 += 6) ST7735_DrawLine(x1, y1, x2, y2, color);
    x2    = w - 1;
    for (y2 = 0; y2 < h; y2 += 6) ST7735_DrawLine(x1, y1, x2, y2, color);

    ST7735_FillScreen(ST7735_BLACK);

    x1    = w - 1;
    y1    = h - 1;
    y2    = 0;
    for (x2 = 0; x2 < w; x2 += 6) ST7735_DrawLine(x1, y1, x2, y2, color);
    x2    = 0;
    for (y2 = 0; y2 < h; y2 += 6) ST7735_DrawLine(x1, y1, x2, y2, color);

}




void testFastLines(uint16_t color1, uint16_t color2)
{
    int           x, y, w = _width, h = _height;

    ST7735_FillScreen(ST7735_BLACK);
//    for (y = 0; y < h; y += 5) drawFastHLine(0, y, w, color1);
    for (y = 0; y < h; y += 5) ST7735_DrawLine(0, y, w-1, y, color1);

//    for (x = 0; x < w; x += 5) drawFastVLine(x, 0, h, color2);
    for (x = 0; x < w; x += 5) ST7735_DrawLine(x, 0, x, h-1, color2);
}





void testRects(uint16_t color)
{
	int           n, i, i2,
	cx = _width  / 2,
	cy = _height / 2;

	ST7735_FillScreen(ST7735_BLACK);
	n     = min(_width, _height);
	for (i = 2; i < n; i += 6) {
		i2 = i / 2;
//        drawRect(cx - i2, cy - i2, i, i, color);
		ST7735_DrawBorder(cx - i2, cy - i2, i, i, 1, color);
	}
}




void testFilledRects(uint16_t color1, uint16_t color2)
{
	int           n, i, i2,
	cx = _width  / 2 - 1,
	cy = _height / 2 - 1;

	ST7735_FillScreen(ST7735_BLACK);
	n = min(_width, _height);
	for (i = n; i > 0; i -= 6) {
		i2    = i / 2;

//       fillRect(cx - i2, cy - i2, i, i, color1);
		ST7735_FillRectangle(cx - i2, cy - i2, i, i, color1);

//        drawRect(cx - i2, cy - i2, i, i, color2);
		ST7735_DrawBorder(cx - i2, cy - i2, i, i, 1,color2);
	}
}




void testFilledCircles(uint8_t radius, uint16_t color)
{
    int x, y, w = _width, h = _height, r2 = radius * 2;

    ST7735_FillScreen(ST7735_BLACK);
    for (x = radius; x < w; x += r2) {
        for (y = radius; y < h; y += r2) {
        	ST7735_FillCircle(x, y, radius, color);
        }
    }
}






void testCircles(uint8_t radius, uint16_t color)
{
    int           x, y, r2 = radius * 2,
                        w = _width  + radius,
                        h = _height + radius;


    for (x = 0; x < w; x += r2) {
        for (y = 0; y < h; y += r2) {
            ST7735_DrawCircle(x, y, radius, color);
        }
    }
}






void testTriangles()
{
    int           n, i, cx = _width  / 2 - 1,
                        cy = _height / 2 - 1;

    ST7735_FillScreen(ST7735_BLACK);
    n     = min(cx, cy);
    for (i = 0; i < n; i += 5) {
    	ST7735_DrawTriangle(
            cx    , cy - i, // peak
            cx - i, cy + i, // bottom left
            cx + i, cy + i, // bottom right
            ST7735_COLOR565(0, 0, i));
    }
}





void testFilledTriangles() {
    int           i, cx = _width  / 2 - 1,
                     cy = _height / 2 - 1;

    ST7735_FillScreen(ST7735_BLACK);
    for (i = min(cx, cy); i > 10; i -= 5) {
    	ST7735_FillTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
    	                         ST7735_COLOR565(0, i, i));
    	ST7735_DrawTriangle(cx, cy - i, cx - i, cy + i, cx + i, cy + i,
    	                         ST7735_COLOR565(i, i, 0));
    }
}






void testRoundRects() {
    int           w, i, i2, red, step,
                  cx = _width  / 2 - 1,
                  cy = _height / 2 - 1;

    ST7735_FillScreen(ST7735_BLACK);
    w     = min(_width, _height);
    red = 0;
    step = (256 * 6) / w;
    for (i = 0; i < w; i += 6) {
        i2 = i / 2;
        red += step;
        ST7735_DrawRoundRect(cx - i2, cy - i2, i, i, i / 8, ST7735_COLOR565(red, 0, 0));
    }
}





void testFilledRoundRects() {
    int           i, i2, green, step,
                  cx = _width  / 2 - 1,
                  cy = _height / 2 - 1;

    ST7735_FillScreen(ST7735_BLACK);
    green = 256;
    step = (256 * 6) / min(_width, _height);
    for (i = min(_width, _height); i > 20; i -= 6) {
        i2 = i / 2;
        green -= step;
        ST7735_FillRoundRect(cx - i2, cy - i2, i, i, i / 8, ST7735_COLOR565(0, green, 0));
    }
}





void TestChar(){

	uint16_t x,y,k,a,b;
	uint8_t c;

	for (k=0;k<2500;k++) {
		a=rand();
		b=rand();
		x=a % (_width-11) ;
		y=b % (_height-18) ;
		c=a % 26+'A';
		ST7735_DrawWChar(x, y, c, Font16, 1, ST7735_YELLOW, ST7735_RED);
		x=b % (_width-11) ;
		y=a % (_height-18) ;
		c=b % 26+'A';
		ST7735_DrawWChar(x, y, c, Font16, 1, ST7735_RED, ST7735_YELLOW);
	}
};



void wait(uint16_t delay){
uint16_t time;
volatile uint32_t dummy1,dummy2;
//volatile uint32_t *mem = 0x20000000;

	time=HAL_GetTick();
	dummy1=0;
	while ((HAL_GetTick()-time)<delay){
		dummy2=dummy1;
		dummy1=dummy2;
	}

}



void TestFillScreen(uint16_t delay) {
	ST7735_FillScreen(ST7735_RED);
	if (delay)
		wait(delay);
    ST7735_FillScreen(ST7735_GREEN);
	if (delay)
		wait(delay);
    ST7735_FillScreen(ST7735_BLUE);
	if (delay)
		wait(delay);
    ST7735_FillScreen(ST7735_YELLOW);
	if (delay)
		wait(delay);
    ST7735_FillScreen(ST7735_BLACK);
	if (delay)
		wait(delay);
}






void TestHVLine() {
	uint16_t k,x,y,l,a,b;
    ST7735_FillScreen(ST7735_BLACK);
    for (k=0 ; k<15000;k++) {
    	a=rand();
    	b=rand();
    	x=a % _width ;
    	y=b % _height ;
    	l=a % (_width - x);
    	ST7735_FillRectangle(x, y, l, 1, ST7735_BLUE);
    	x=b % _width ;
    	y=a % _height ;
    	l=b % (_height - y);
    	ST7735_FillRectangle(x, y, 1, l, ST7735_CYAN);
    }
}






/* @brief private function for TestDisplay() */
void Displ_Page(char * str1,char * str2,char * str3, uint8_t mode) {
const uint16_t bcol0=ST7735_BLACK, col1=ST7735_WHITE, col2=ST7735_WHITE, col3=ST7735_WHITE, bcol1=ST7735_BLUE, bcol2=ST7735_BLACK, bcol3=ST7735_BLACK;
	ST7735_FillRectangle(0, 21, _width, 72, bcol0);
	ST7735_DrawCSString(0,21, _width, 21+24, str1, Font24, 1, col1,bcol1);
	ST7735_DrawCSString(0,54, _width, 54+16, str2, Font16, 1, col2,bcol2);
	ST7735_DrawCSString(0,77, _width, 77+16, str3, Font16, 1, col3,bcol3);
}







void testFillScreen()
{
	ST7735_FillScreen(ST7735_RED);
	ST7735_FillScreen(ST7735_GREEN);
	ST7735_FillScreen(ST7735_BLUE);
	ST7735_FillScreen(ST7735_YELLOW);
	ST7735_FillScreen(ST7735_BLACK);
}






void Displ_ColorTest(){
	const uint8_t colnum=8;
	const uint8_t rownum=3;
	const uint16_t colortab[]={ST7735_WHITE,ST7735_RED,ST7735_BLUE,ST7735_GREEN,ST7735_YELLOW,ST7735_MAGENTA,ST7735_CYAN,ST7735_WHITE, \
								ST7735_WHITE,ST7735_RED,ST7735_BLUE,ST7735_GREEN,ST7735_YELLOW,ST7735_MAGENTA, ST7735_CYAN, ST7735_WHITE, \
								ST7735_WHITE,ST7735_RED,ST7735_BLUE,ST7735_GREEN,ST7735_YELLOW,ST7735_MAGENTA,ST7735_CYAN,ST7735_WHITE};
	static ST7735_Orientat_t orientation = Displ_Orientat_90;
	uint16_t x,y,dx,dy;
	ST7735_SetRotation(orientation);
	if (orientation==Displ_Orientat_0)
		orientation=Displ_Orientat_90;
	else
		orientation=Displ_Orientat_0;
	dx=_width/colnum;
	dy=_height/rownum;
	for(y=0; y<rownum;y++){
		for(x=0; x<colnum; x++){
			ST7735_FillRectangle(x*dx, y*dy, dx, dy, colortab[y*colnum+x]);
		}
		__NOP();
	}
	if ((x*dx)<_width)
		ST7735_FillRectangle(x*dx, 0, (_width-x*dx), _height, ST7735_BLACK);
	if ((y*dy)<_height)
		ST7735_FillRectangle(0, y*dy, _width, (_height-y*dy), ST7735_BLACK);
}







void Displ_TestAll (){
	testFillScreen();
	testLines(ST7735_CYAN);
	testFastLines(ST7735_RED, ST7735_BLUE);
	testRects(ST7735_GREEN);
	testFilledRects(ST7735_YELLOW, ST7735_MAGENTA);
	testFilledCircles(10, ST7735_MAGENTA);
	testCircles(10, ST7735_WHITE);
	testTriangles();
	testFilledTriangles();
	testRoundRects();
	testFilledRoundRects();
}





void Displ_PerfTest() {
	  uint32_t time[6];
		uint32_t time1 = 0, time2 = 0;
	  char riga[40];
	  uint8_t k;

	  time1 = HAL_GetTick();
	  Displ_TestAll();
		time2 = HAL_GetTick();
		time[1] = time2 - time1;
	  sprintf(riga,"%u ms",time[1]);
	  Displ_Page("TEST 1","TestAll:",riga,0);
	  HAL_Delay(3000);

	  time[2] = HAL_GetTick();
	  for (k=0;k<10;k++)
		  TestFillScreen(0);
	  time[2] = HAL_GetTick()-time[2];
	  sprintf(riga,"%u ms",time[2]);
	  Displ_Page("TEST 2","50 screens:",riga,0);
	  HAL_Delay(3000);

	  time[3] = HAL_GetTick();
	  TestHVLine();
	  time[3] = HAL_GetTick()-time[3];
	  sprintf(riga,"%u ms",time[3]);
	  Displ_Page("TEST 3","30k lines:",riga,0);
	  HAL_Delay(3000);

	  ST7735_FillScreen(ST7735_BLACK);
	  time[4] = HAL_GetTick();
	  TestChar();
	  time[4] = HAL_GetTick()-time[4];
	  sprintf(riga,"%u ms",time[4]);
	  Displ_Page("TEST 4","5000 chars:",riga,0);
	  HAL_Delay(3000);


	  ST7735_FillScreen(BGCOLOR);

	  ST7735_FillRectangle(0, 0, _width, 135, BGCOLOR);
	  ST7735_DrawCSString(0,0,_width,38, "RESULTS", Font24, 1, ST7735_WHITE,ST7735_BLUE);

	for (uint8_t k=1;k<5;k++) {
		switch (k) {
			case 0:
//				sprintf(riga,"INITIAL SETUP: %ld ms",time[k]);
				break;
			case 1:
				sprintf(riga,"STD TEST      %4u",time[k]);
				break;
			case 2:
				sprintf(riga,"50 SCREENS    %4u",time[k]);
				break;
			case 3:
				sprintf(riga,"30k H/V LINES %4u",time[k]);
				break;
			case 4:
				sprintf(riga,"5k CHARS      %4u",time[k]);
				break;
		}
		ST7735_DrawWString(0, 25+(k)*Font12.Height*2, riga, Font12, 1, ST7735_WHITE, BGCOLOR);
	};

#ifdef DISPLAY_DIMMING_MODE

	{
		uint32_t maxlevel=ST7735_SetBacklight('F'); //set display backlight 100% and get the corresponding value (that means ARR)
		uint16_t dtime=4000/(maxlevel*3);
		if (dtime==0)
			dtime=1;
		ST7735_SetBacklight('0');  // turn off display

		for (uint32_t k=0; k<=maxlevel;k++){
			ST7735_SetBacklight('+');  // increase light
			HAL_Delay(dtime);
		}
		HAL_Delay(100);
		for (uint32_t k=0; k<=maxlevel;k++){
			ST7735_SetBacklight('-');  // decrease light
			HAL_Delay(dtime);
		}
		HAL_Delay(100);
		for (uint32_t k=0; k<=maxlevel;k++){
			ST7735_SetBacklight('+');  // increase light
			HAL_Delay(dtime);
		}
		HAL_Delay(1000);
	}

#else
	HAL_Delay(5000);
#endif
}
