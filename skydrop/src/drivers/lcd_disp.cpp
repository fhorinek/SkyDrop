#include "lcd_disp.h"
#include "uart.h"


void lcd_display::SetDrawLayer(uint8_t layer)
{
	this->active_buffer = this->layers[layer];
}

/**
 * Set on screen position for next character
 *
 */
void lcd_display::GotoXY(uint8_t x, uint16_t y)
{
	text_x = x;
	text_y = y;
}

/**
 * Write ASCII character on screen
 *
 */
void lcd_display::Write(uint8_t ascii=0)
{
	if (text_x >= lcd_width)
		return;

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
//			if (text_x >= lcd_width)
//			{
//				text_x = 0;
//				text_y += this->font_height;
//			}
		}
	}

	text_x += font_spacing;
//	if (text_x >= lcd_width)
//	{
//		text_x = 0;
//		text_y += this->font_height;
//	}
}

uint8_t lcd_display::GetTextWidth(char * text)
{
	uint8_t ret = 0;

	while (*text != 0)
	{
		if (*text < font_begin || *text > font_end)
		{
			ret += font_spacing * 2;
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

uint8_t lcd_display::GetTextWidthN(char * text, uint8_t n)
{
	uint8_t ret = 0;

	for (uint8_t i = 0; i < n && *text != 0; i++)
	{
		if (*text < font_begin || *text > font_end)
		{
			ret += font_spacing * 2;
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

uint8_t lcd_display::GetTextHeight()
{
	return this->font_height;
}

uint8_t lcd_display::GetAHeight()
{
	return this->font_A_height;
}


void lcd_display::LoadFont(const uint8_t * font)
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


void lcd_display::sendcommand(unsigned char cmd)
{
	GpioWrite(LCD_DC, LOW);

	this->spi->SetSlave(LCD_CE);
	this->spi->SendRaw(cmd);
	this->spi->UnsetSlave();
}

/**
 * Set display to active mode
 */
void lcd_display::Init(Spi * spi)
{
	this->spi = spi;

	CreateSinTable();


	GpioSetDirection(LCD_RST, OUTPUT);
	GpioSetDirection(LCD_DC, OUTPUT);
	GpioSetDirection(LCD_CE, OUTPUT);
	GpioSetDirection(LCD_VCC, OUTPUT);

	GpioWrite(LCD_VCC, HIGH);

	GpioWrite(LCD_RST, LOW);
	_delay_ms(10);
	GpioWrite(LCD_RST, HIGH);

	#define START_LINE	64

	sendcommand(0x21); //Extended commands

	sendcommand(0x04 | (0b01000000 & START_LINE) >> 6);
	sendcommand(0x40 | (0b00111111 & START_LINE));

//	sendcommand(0x04); //Set Temp coefficent
	sendcommand(0x13); //LCD bias mode 1:48: Try 0x13 or 0x14

	this->SetContrast(72);

	sendcommand(0x20); //Horizontal addressing mode.
	this->SetInvert(false);

	for (uint8_t i = 0; i < DISP_LAYERS; i++)
	{
		SetDrawLayer(i);
		ClearBuffer();
	}

	SetDrawLayer(0);

	Draw();
}

void lcd_display::SetContrast(uint8_t val) //0-127
{
	sendcommand(0x21); //Extended
	if (val > 81)
		val = 81;

	sendcommand(0x80 | val);
	sendcommand(0x20); //Basic
}

void lcd_display::SetInvert(uint8_t invert)
{
	 //Set display control, normal mode. 0x0D for inverse
	if (invert)
		sendcommand(0x0D);
	else
		sendcommand(0x0C);
}

void lcd_display::SetFlip(bool flip)
{
	this->flip = flip;
}


void lcd_display::Stop()
{
	GpioSetDirection(LCD_RST, INPUT);
	GpioSetDirection(LCD_DC, INPUT);
	GpioSetDirection(LCD_CE, INPUT);
	GpioSetDirection(LCD_VCC, INPUT);
}

/**
 * Draw line (works in any direction)
 *
 */
void lcd_display::DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t color=1){
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
void lcd_display::PutPixel(uint8_t x ,uint16_t  y ,uint8_t color)
{
	if (x >= lcd_width || y >= lcd_height )
		return;

	uint16_t index = ((y / 8) * lcd_width) + (x % lcd_width);
	if (color == DISP_COLOR_WHITE)
		active_buffer[index] |= (1 << (y % 8));
	else {
		active_buffer[index] &= ~(1 << (y % 8));
	}
}

void lcd_display::InvertPixel(uint8_t x ,uint8_t  y)
{
	if (x >= lcd_width || y >= lcd_height)
		return;

	uint16_t index = ((y / 8) * lcd_width) + (x % lcd_width);
	active_buffer[index] ^= (1 << (y % 8));
}

void lcd_display::DrawImage(const uint8_t *data,uint8_t x,uint8_t y)
{
	uint8_t cbuf;

	cbuf = pgm_read_byte(&data[0]);

	uint8_t imgwidth = (cbuf+x < lcd_width)?cbuf:lcd_width-x;
	int16_t xCutOff  = (cbuf+x < lcd_width)?0:(cbuf+x-lcd_width);
	uint8_t yOffset  = (y/8 < 1)?0:y/8;

	cbuf = pgm_read_byte(&data[1]);

	uint8_t imgheight = (cbuf/8);
	uint8_t _x = x;
	uint8_t _y = 0;
	uint16_t index = 2;

	if (y >= lcd_height || x >= lcd_width) return;

	for (_y=0;_y < imgheight; _y++){
		for(_x=0;_x < imgwidth; _x++){

			cbuf = pgm_read_byte(&data[index]);

			if (y % 8 != 0) {
				uint16_t tmp = 0;

				uint8_t  tmpdat = cbuf;
				tmp = (tmpdat << (y%8));

				active_buffer[((_y+yOffset)*lcd_width)+_x+x] |= (tmp & 255);
				if (_y+yOffset+1 < lcd_height)
				active_buffer[((_y+yOffset+1)*lcd_width)+_x+x] |= ((tmp >> 8) & 255);
			}
			else
				active_buffer[((_y+yOffset)*lcd_width)+_x+x] |= cbuf;
			index++;
		}
		index += xCutOff;
	}
}

void lcd_display::DrawRectangle(int8_t x1,int8_t y1,int8_t x2,int8_t y2,uint8_t color=1,uint8_t fill=0){
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

void lcd_display::Invert(int8_t x1,int8_t y1,int8_t x2,int8_t y2){
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

void lcd_display::InvertPart(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2)
{
	uint8_t y, x;

  for(y = y1;y <= y2; y++)
  {
	  for (x = x1; x < x2; x++)
	  {
		  active_buffer[lcd_width * y + x] ^= 0xFF;
	  }
  }
}

void lcd_display::DrawArc(uint8_t cx,uint8_t cy,uint8_t radius,int16_t start,int16_t end)
{
	int16_t angle = 0;
	int8_t x,y;

	for (angle=start;angle<=end;angle++)
	{
		x = radius * get_sin(angle);
		y = radius * get_sin(angle+180);
		PutPixel(cx+x,cy + y,1);
	}
}

void lcd_display::DrawTriangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t x3,uint8_t y3,uint8_t color){
	 DrawLine(x1, y1, x2,y2,color);
	 DrawLine(x1,y1,x3,y3,color);
	 DrawLine(x2,y2,x3,y3,color);
}

void lcd_display::DrawCircle(uint8_t cx, uint8_t cy, uint8_t radius,uint8_t color=1)
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

void lcd_display::plot8points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y,uint8_t color)
{
  plot4points(cx, cy, x, y, color);
  if (x != y) plot4points(cx, cy, y, x, color);
}

void lcd_display::plot4points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y,uint8_t color)
{
  PutPixel(cx + x, cy + y,color);
  if (x != 0) PutPixel(cx - x, cy + y,color);
  if (y != 0) PutPixel(cx + x, cy - y,color);
  if (x != 0 && y != 0) PutPixel(cx - x, cy - y,color);
}

void lcd_display::SetRowCol(unsigned char row, unsigned char col)
{
	this->sendcommand(0x80 | col);
	this->sendcommand(0x40 | row);
}


void lcd_display::Draw()
{
	if (this->flip)
	{
		for (uint8_t j=0;j<6;j++)
		{
			SetRowCol(5 - j, 0);

			GpioWrite(LCD_DC, HIGH);
			this->spi->SetSlave(LCD_CE);
			for (uint8_t a=0; a < lcd_width; a++)
			{
				this->spi->SendRaw(fast_flip(active_buffer[lcd_width - 1 - a + (j * lcd_width)]));
			}
			this->spi->UnsetSlave();
		}
	}
	else
	{
		for (uint8_t j=0;j<6;j++)
		{
			SetRowCol(j, 0);

			GpioWrite(LCD_DC, HIGH);
			this->spi->SetSlave(LCD_CE);
			for (uint8_t a=0; a < lcd_width; a++)
			{
				this->spi->SendRaw(active_buffer[a+(j * lcd_width)]);
			}
			this->spi->UnsetSlave();
		}
	}
}

void lcd_display::CopyToLayerX(uint8_t dst, int8_t x)
{
	uint8_t start_x, end_x, col_x;

	if (x < 0)
	{
		start_x = abs(x);
		end_x = lcd_width;
		col_x = 0;
	}
	else
	{
		start_x = 0;
		end_x = lcd_width - x;
		col_x = x;
	}

	for (uint8_t j=0;j<6;j++)
	{
		uint16_t index = j * lcd_width + col_x;
		uint8_t cnt = 0;

		for (uint8_t a = start_x; a < end_x; a++)
		{
			this->layers[dst][index + cnt] = this->active_buffer[a + (j * lcd_width)];
			cnt++;
		}
	}
}

void lcd_display::CopyToLayerXPart(uint8_t dst, int8_t x, uint8_t row1, uint8_t row2)
{
	uint8_t start_x, end_x, col_x;

	if (x < 0)
	{
		start_x = abs(x);
		end_x = lcd_width;
		col_x = 0;
	}
	else
	{
		start_x = 0;
		end_x = lcd_width - x;
		col_x = x;
	}

	for (uint8_t j=row1; j<row2; j++)
	{
		uint16_t index = j * lcd_width + col_x;
		uint8_t cnt = 0;

		for (uint8_t a = start_x; a < end_x; a++)
		{
			this->layers[dst][index + cnt] = this->active_buffer[a + (j * lcd_width)];
			cnt++;
		}
	}
}

void lcd_display::CopyToLayer(uint8_t dst)
{
	memcpy(this->layers[dst], this->active_buffer, (lcd_height / 8) * lcd_width);
}

uint8_t * lcd_display::GetActiveLayerPtr()
{
	return this->active_buffer;
}

void lcd_display::CopyToLayerPart(uint8_t dst, uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2)
{
	for (uint8_t j=row1; j<row2; j++)
	{
		uint16_t start_i = j * lcd_width;

		for (uint8_t a = col1; a < col2; a++)
		{
			uint16_t index = start_i + a;
			this->layers[dst][index] = this->active_buffer[index];
		}
	}
}

void lcd_display::ClearBuffer(void){
  unsigned char i,k;
  for(k=0;k<6;k++)
  {
	  for(i=0;i<lcd_width;i++)     //clear all COL
	  {
		active_buffer[i+(k*lcd_width)] = 0;
	  }
  }
}

void lcd_display::ClearPart(uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2)
{
	for (uint8_t j = row1; j < row2; j++)
	{
		uint16_t index = j * lcd_width;
		uint8_t cnt = 0;

		for (uint8_t a = col1; a < col2; a++)
		{
			this->active_buffer[index + cnt] = 0;
			cnt++;
		}
	}
}

void  lcd_display::CreateSinTable(){
	  for (int16_t i=0; i < 91; i++)
		  sin_table[i] = sin(((float)i/180.0)*3.142);
}

float lcd_display::get_sin(uint16_t angle)
{
	angle = angle % 360;

	if (angle < 90)
		return this->sin_table[angle];
	else if (angle < 180)
		return this->sin_table[90 - (angle - 90)];
	else if (angle < 270)
		return -this->sin_table[angle - 180];
	else return -this->sin_table[90 - (angle - 270)];
}

float lcd_display::get_cos(uint16_t angle)
{
	angle += 270;
	return this->get_sin(angle);
}
