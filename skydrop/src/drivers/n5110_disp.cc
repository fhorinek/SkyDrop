#include "n5110_disp.h"
#include "uart.h"


void n5110display::SetDrawLayer(uint8_t layer)
{
	this->active_buffer = this->layers[layer];
}

/**
 * Set on screen position for next character
 *
 */
void n5110display::GotoXY(uint8_t x, uint8_t y)
{
	text_x = x;
	text_y = y;
}

/**
 * Write ASCII character on screen
 *
 */
void n5110display::Write(uint8_t ascii=0)
{
	if (ascii < font_begin || ascii > font_end)
	{
		text_x += font_spacing;
	}
	else
	{
		uint16_t adr = 6 + (ascii - font_begin) * 2;

		uint16_t start = pgm_read_word(&this->font_data[adr]);
		uint16_t width = pgm_read_word(&this->font_data[adr + 2]) - start;

		adr = this->font_adr_start + start * this->font_lines;

		for (uint8_t x = 0; x < width; x++)
		{
			uint16_t index = adr + x * font_lines;
			for (uint8_t n = 0; n < font_lines; n++)
			{
				uint8_t data = pgm_read_byte(&this->font_data[index + n]);

				for (uint8_t a = 0; a < 8; a++)
				{
					if (data & (1 << a))
						this->PutPixel(text_x, text_y + a + n * 8, 1);
				}
			}

			text_x++;
			if (text_x >= n5110_width)
			{
				text_x = 0;
				text_y += this->font_height;
			}
		}
	}

	text_x += font_spacing;
	if (text_x >= n5110_width)
	{
		text_x = 0;
		text_y += this->font_height;
	}
}

uint8_t n5110display::GetTextWidth(char * text)
{
	uint8_t ret = 0;

	while (*text != 0)
	{
		if (*text < font_begin || *text > font_end)
		{
			ret += font_spacing;
		}
		else
		{
			uint16_t adr = 6 + (*text - font_begin) * 2;

			uint16_t start = pgm_read_word(&this->font_data[adr]);
			uint8_t width = pgm_read_word(&this->font_data[adr + 2]) - start;

			ret += font_spacing + width;
		}
		text++;
	}

	return ret;
}

uint8_t n5110display::GetTextHeight()
{
	return this->font_height;
}

uint8_t n5110display::GetAHeight()
{
	return this->font_A_height;
}


void n5110display::LoadFont(const uint8_t * font)
{
	this->font_data = font;
	this->font_height = pgm_read_byte(&font[0]);
	this->font_A_height = pgm_read_byte(&font[1]);
	this->font_spacing = pgm_read_byte(&font[2]);
	this->font_lines = pgm_read_byte(&font[3]);
	this->font_begin = pgm_read_byte(&font[4]);
	this->font_end = pgm_read_byte(&font[5]);

	this->font_adr_start = 6; //header
	this->font_adr_start += (this->font_end - this->font_begin + 2) * 2; //char adr table
}


void n5110display::sendcommand(unsigned char cmd)
{
	GpioWrite(LCD_DC, LOW);

	this->spi->SetSlave(LCD_CE);
	this->spi->SendRaw(cmd);
	this->spi->UnsetSlave();
}

/**
 * Set display to active mode
 *
 * \param i2c Pointer to i2c object
 */
void n5110display::Init()
{
	LCD_SPI_PWR_ON;

	this->spi = new Spi;

	this->spi->InitMaster(LCD_SPI);
	this->spi->SetDivider(spi_div_64);
	this->spi->SetDataOrder(MSB);

//	CreateSinTable();


	GpioSetDirection(LCD_RST, OUTPUT);
	GpioSetDirection(LCD_DC, OUTPUT);
	GpioSetDirection(LCD_CE, OUTPUT);
	GpioSetDirection(LCD_VCC, OUTPUT);

	GpioWrite(LCD_VCC, HIGH);

	GpioWrite(LCD_RST, LOW);
	_delay_ms(10);
	GpioWrite(LCD_RST, HIGH);


	sendcommand(0x21); //Extended
	sendcommand(0x04); //Set Temp coefficent
	sendcommand(0x13); //LCD bias mode 1:48: Try 0x13 or 0x14

	this->SetContrast(72);

	sendcommand(0x20); //Basic
	sendcommand(0x0C); //Set display control, normal mode. 0x0D for inverse

	for (uint8_t i = 0; i < DISP_LAYERS; i++)
	{
		SetDrawLayer(i);
		ClearBuffer();
	}

	SetDrawLayer(0);

	Draw();
}

void n5110display::SetContrast(uint8_t val) //0-127
{
	sendcommand(0x21); //Extended
	sendcommand(0x80 | val); //Set LCD Vop (Contrast): Try 0xB1(good @ 3.3V) or 0xBF if your display is too dark
	sendcommand(0x20); //Basic
}

void n5110display::Stop()
{
	this->spi->Stop();
	delete this->spi;

	GpioSetDirection(LCD_RST, INPUT);
	GpioSetDirection(LCD_DC, INPUT);
	GpioSetDirection(LCD_CE, INPUT);
	GpioSetDirection(LCD_VCC, INPUT);

	LCD_SPI_PWR_OFF;
}

/**
 * Send pixel data to display
 *
 */
void n5110display::SendChar(unsigned char data)
{
	GpioWrite(LCD_DC, HIGH);

	this->spi->SetSlave(LCD_CE);
	this->spi->SendRaw(data);
	this->spi->UnsetSlave();
}

/**
 * Draw line (works in any direction)
 *
 */
void n5110display::DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t color=1){
	uint8_t deltax,deltay,x,y,xinc1,xinc2,yinc1,yinc2,den,num,numadd,numpixels,curpixel;
	deltax = abs(x2 - x1);		// The difference between the x's
	deltay = abs(y2 - y1);		// The difference between the y's
	x = x1;				   	// Start x off at the first pixel
	y = y1;				   	// Start y off at the first pixel

	if (x2 >= x1){			 	// The x-values are increasing
	  xinc1 = 1;
	  xinc2 = 1;
	}
	else{						  // The x-values are decreasing
	  xinc1 = -1;
	  xinc2 = -1;
	}

	if (y2 >= y1){			 	// The y-values are increasing
	  yinc1 = 1;
	  yinc2 = 1;
	}
	else{						  // The y-values are decreasing
	  yinc1 = -1;
	  yinc2 = -1;
	}

	if (deltax >= deltay){	 	// There is at least one x-value for every y-value
	  xinc1 = 0;				  // Don't change the x when numerator >= denominator
	  yinc2 = 0;				  // Don't change the y for every iteration
	  den = deltax;
	  num = deltax / 2;
	  numadd = deltay;
	  numpixels = deltax;	 	// There are more x-values than y-values
	}
	else{						  // There is at least one y-value for every x-value
	  xinc2 = 0;				  // Don't change the x for every iteration
	  yinc1 = 0;				  // Don't change the y when numerator >= denominator
	  den = deltay;
	  num = deltay / 2;
	  numadd = deltax;
	  numpixels = deltay;	 	// There are more y-values than x-values
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++){
	  PutPixel(x, y,color);		 	// Draw the current pixel
	  num += numadd;			  // Increase the numerator by the top of the fraction
	  if (num >= den){		 	// Check if numerator >= denominator
		num -= den;		   	// Calculate the new numerator value
		x += xinc1;		   	// Change the x as appropriate
		y += yinc1;		   	// Change the y as appropriate
	  }
	  x += xinc2;			 	// Change the x as appropriate
	  y += yinc2;			 	// Change the y as appropriate
	}
}

/**
 * Put pixel on screen
 *
 * /param val -
 */
void n5110display::PutPixel(uint8_t x ,uint8_t  y ,uint8_t color)
{
	if (x >= n5110_width || y >= n5110_height)
		return;

	uint16_t index = ((y / 8) * n5110_width) + (x % n5110_width);
	if (color == DISP_COLOR_WHITE)
		active_buffer[index] |= (1 << (y % 8));
	else {
		active_buffer[index] &= ~(1 << (y % 8));
	}
}

void n5110display::InvertPixel(uint8_t x ,uint8_t  y)
{
	if (x >= n5110_width || y >= n5110_height)
		return;

	uint16_t index = ((y / 8) * n5110_width) + (x % n5110_width);
	active_buffer[index] ^= (1 << (y % 8));
}

void n5110display::DrawImage(uint8_t *data,uint8_t x,uint8_t y){
	uint8_t imgwidth = (data[0]+x < 128)?data[0]:128-x;
	int16_t xCutOff  = (data[0]+x < 128)?0:(data[0]+x-128);
	uint8_t yOffset  = (y/8 < 1)?0:y/8;
	uint8_t imgheight = (data[1]/8);
	uint8_t _x = x;
	uint8_t _y = 0;
	uint16_t index = 2;

	if (y >= 64 || x >= 128) return;

	for (_y=0;_y < imgheight; _y++){
		for(_x=0;_x < imgwidth; _x++){
			if (y % 8 != 0) {
				uint16_t tmp = 0;
				uint8_t  tmpdat = data[index];
				tmp = (tmpdat << (y%8));

				active_buffer[((_y+yOffset)*128)+_x+x] |= (tmp & 255);
				if (_y+yOffset+1 < 64)
				active_buffer[((_y+yOffset+1)*128)+_x+x] |= ((tmp >> 8) & 255);
			}
			else
				active_buffer[((_y+yOffset)*128)+_x+x] |= data[index];
			index++;
		}
		index += xCutOff;
	}
}

void n5110display::DrawRectangle(int8_t x1,int8_t y1,int8_t x2,int8_t y2,uint8_t color=1,uint8_t fill=0){
	int8_t xdir;
	int8_t xref = (x1 <= x2) ? x2:x1;
	int8_t ydir = (y1 <= y2) ? y2:y1;
	int8_t yref = (y1 <= y2) ? y1:y2;

	for(; ydir >= yref; ydir--){
		PutPixel(x1,ydir,color);
		PutPixel(x2,ydir,color);
		if (fill==0 && (ydir != y1 && ydir != y2)) continue;
		for(xdir = (x1 <= x2) ? (x1+1):(x2+1); xdir <= (xref-1); xdir++){
			PutPixel(xdir,ydir,color);
		}
	}
}

void n5110display::Invert(int8_t x1,int8_t y1,int8_t x2,int8_t y2){
	int8_t xdir;
	int8_t xref = (x1 <= x2) ? x2:x1;
	int8_t ydir = (y1 <= y2) ? y2:y1;
	int8_t yref = (y1 <= y2) ? y1:y2;

	for(; ydir >= yref; ydir--){
		InvertPixel(x1,ydir);
		InvertPixel(x2,ydir);
		for(xdir = (x1 <= x2) ? (x1+1):(x2+1); xdir <= (xref-1); xdir++){
			InvertPixel(xdir,ydir);
		}
	}
}

void n5110display::InvertPart(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
	uint8_t y, x;

  for(y = y1;y <= y2; y++)
  {
	  for (x = x1 * 8; x < x2 * 8; x++)
	  {
		  active_buffer[16 * 8 * y + x] ^= 0xFF;
	  }
  }
}

void n5110display::DrawArc(uint8_t cx,uint8_t cy,uint8_t radius,int16_t start,int16_t end)
{
	int16_t angle = 0;
	int8_t x,y;

	for (angle=start;angle<=end;angle++)
	{
//		x = radius * sin_table[(angle+90)%360];
//		y = radius * sin_table[(angle+180)%360];
		PutPixel(cx+x,cy + y,1);
	}
}

void n5110display::DrawTriangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t x3,uint8_t y3,uint8_t color){
	 DrawLine(x1, y1, x2,y2,color);
	 DrawLine(x1,y1,x3,y3,color);
	 DrawLine(x2,y2,x3,y3,color);
}

void n5110display::DrawCircle(uint8_t cx, uint8_t cy, uint8_t radius,uint8_t color=1)
{
	int8_t error = -radius;
	uint8_t x = radius;
	uint8_t y = 0;

  while (x >= y)
  {
    plot8points(cx, cy, x, y, color);

    error += y;
    ++y;
    error += y;

    if (error >= 0)
    {
      error -= x;
      --x;
      error -= x;
    }
  }
}

void n5110display::plot8points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y,uint8_t color)
{
  plot4points(cx, cy, x, y, color);
  if (x != y) plot4points(cx, cy, y, x, color);
}

void n5110display::plot4points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y,uint8_t color)
{
  PutPixel(cx + x, cy + y,color);
  if (x != 0) PutPixel(cx - x, cy + y,color);
  if (y != 0) PutPixel(cx + x, cy - y,color);
  if (x != 0 && y != 0) PutPixel(cx - x, cy - y,color);
}

void n5110display::SetRowCol(unsigned char row,unsigned char col)
{
	this->sendcommand(0x80 | col);
	this->sendcommand(0x40 | row);
}

void n5110display::Draw()
{
	for (uint8_t j=0;j<6;j++)
	{
		SetRowCol(j, 0);

		GpioWrite(LCD_DC, HIGH);
		this->spi->SetSlave(LCD_CE);
		for (uint8_t a=0;a<n5110_width;a++)
		{
			this->spi->SendRaw(active_buffer[a+(j*n5110_width)]);
		}
		this->spi->UnsetSlave();
	}
}

void n5110display::CopyToLayerX(uint8_t dst, int8_t x)
{
	uint8_t start_x, end_x, col_x;

	if (x < 0)
	{
		start_x = abs(x);
		end_x = n5110_width;
		col_x = 0;
	}
	else
	{
		start_x = 0;
		end_x = n5110_width - x;
		col_x = x;
	}

	for (uint8_t j=0;j<6;j++)
	{
		uint16_t index = j * n5110_width + col_x;
		uint8_t cnt = 0;

		for (uint8_t a = start_x; a < end_x; a++)
		{
			this->layers[dst][index + cnt] = this->active_buffer[a + (j * n5110_width)];
			cnt++;
		}
	}
}

void n5110display::CopyToLayerPart(uint8_t dst, uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2)
{
	for (uint8_t j=row1;j<row2;j++)
	{
		uint16_t start_i = j * n5110_width;

		for (uint8_t a = col1; a < col2; a++)
		{
			uint16_t index = start_i + a;
			this->layers[dst][index] = this->active_buffer[index];
		}
	}
}

void n5110display::ClearBuffer(void){
  unsigned char i,k;
  for(k=0;k<6;k++)
  {
	{
	  for(i=0;i<n5110_width;i++)     //clear all COL
	  {
		active_buffer[i+(k*n5110_width)] = 0;
	  }
	}
  }
}

void n5110display::ClearPart(uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2)
{
	for (uint8_t j = row1; j < row2; j++)
	{
		uint16_t index = j * n5110_width;
		uint8_t cnt = 0;

		for (uint8_t a = col1; a < col2; a++)
		{
			this->active_buffer[index + cnt] = 0;
			cnt++;
		}
	}
}

//void  n5110display::CreateSinTable(){
//	  for (int16_t i=0; i < 360; i++)
//		  sin_table[i] = sin(((float)i/180.0)*3.142);
//}
