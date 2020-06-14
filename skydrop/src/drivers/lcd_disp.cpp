#include "lcd_disp.h"
#include "uart.h"

#include "debug_on.h"

void lcd_display::SetDrawLayer(uint8_t layer)
{
	this->active_buffer = this->layers[layer];
}

/**
 * Set on screen position for next character
 *
 */
void lcd_display::GotoXY(int8_t x, int8_t y)
{
	text_x = x;
	text_y = y;
}

void lcd_display::GotoXY_16(int16_t x, int16_t y)
{
	text_x = x;
	text_y = y;
}
void lcd_display::clip(uint8_t x1, uint8_t x2)
{
	clip_x1 = x1;
	clip_x2 = x2;
}

void lcd_display::noclip()
{
	clip_x1 = 0;
	clip_x2 = lcd_width;
}
/**
 * Write ASCII character on screen
 *
 */
void lcd_display::Write(uint8_t ascii = 0)
{
	if (text_x >= lcd_width)
		return;

	if (ascii < font_begin || ascii > font_end)
	{
		text_x += font_spacing;
	}
	else
	{
		uint8_t char_index = ascii - font_begin;
		uint16_t adr = 7 + char_index;

		uint16_t start = pgm_read_byte(&this->font_data[adr]) + (char_index >= font_9bit ? 0xFF : 0) ;
		uint16_t width = pgm_read_byte(&this->font_data[adr + 1]) + (char_index + 1 >= font_9bit ? 0xFF : 0) - start;

		adr = this->font_adr_start + start * this->font_lines;

		for (uint8_t x = 0; x < width; x++)
		{
			uint16_t index = adr + x * font_lines;

			if (text_x < lcd_width && text_x >= 0)
			{
				for (uint8_t n = 0; n < font_lines; n++)
				{
					uint8_t data = pgm_read_byte(&this->font_data[index + n]);

					for (uint8_t a = 0; a < 8; a++)
					{
						if (data & (1 << a))
							this->PutPixel(text_x, text_y + a + n * 8, 1);
					}
				}
			}

			text_x++;
		}
	}

	text_x += font_spacing;
}

uint16_t lcd_display::GetTextWidth_P(const char * text)
{
	char textRAM[100];

	strcpy_P(textRAM, text);
	return GetTextWidth(textRAM);
}

uint16_t lcd_display::GetTextWidth(char * text, uint16_t n)
{
	uint16_t ret = 0;

	for (uint16_t i = 0; i < n && *text != 0; i++)
	{
		if (*text < font_begin || *text > font_end)
		{
			ret += font_spacing * 2;
		}
		else
		{
			uint8_t char_index = *text - font_begin;
			uint16_t adr = 7 + char_index;

			uint16_t start = pgm_read_byte(&this->font_data[adr]) + (char_index >= font_9bit ? 0xFF : 0) ;
			uint16_t width = pgm_read_byte(&this->font_data[adr + 1]) + (char_index + 1 >= font_9bit ? 0xFF : 0) - start;

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
	this->font_9bit = pgm_read_byte(&font[6]);

	this->font_adr_start = 7; //header
	this->font_adr_start += this->font_end - this->font_begin + 2; //char adr table
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
	this->noclip();

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

	if (val > 127)
		val = 127;

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
void lcd_display::DrawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint8_t color)
{
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy, e2; // error value e_xy

	while (1)
	{
		PutPixel(x0, y0, color);
		if (x0 == x1 && y0 == y1)
			break;
		e2 = 2 * err;
		if (e2 > dy)
		{
			err += dy;
			x0 += sx;
		} // e_xy+e_x > 0
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		} // e_xy+e_y < 0
	}
}

/**
 * Put pixel on screen
 *
 * /param x the x position in pixel
 * /param y the y position in pixel
 * /param color the color which could be either DISP_COLOR_WHITE or "0" for black
 */
void lcd_display::PutPixel(int8_t x, int8_t y, uint8_t color)
{
	if (x < clip_x1 || x >= clip_x2 || y >= lcd_height || y < 0)
		return;

	uint16_t index = ((y / 8) * lcd_width) + (x % lcd_width);

	if (color == DISP_COLOR_BLACK)
	{
		active_buffer[index] |= (1 << (y % 8));
	}
	else
	{
		active_buffer[index] &= ~(1 << (y % 8));
	}
}

void lcd_display::InvertPixel(uint8_t x, uint8_t y)
{
	if (x >= lcd_width || y >= lcd_height)
		return;

	uint16_t index = ((y / 8) * lcd_width) + (x % lcd_width);
	active_buffer[index] ^= (1 << (y % 8));
}

void lcd_display::DrawImage(const uint8_t *data, uint8_t x, uint8_t y)
{
	uint8_t w, h;

	w = pgm_read_byte(&data[0]);
	h = pgm_read_byte(&data[1]);
	DrawImage(data + 2, x, y, w, h);
}

void lcd_display::DrawImage(const uint8_t *data, uint8_t x, uint8_t y,
		uint8_t w, uint8_t h)
{
	uint8_t cbuf;

	uint8_t imgwidth = (w + x < lcd_width) ? w : lcd_width - x;
	int16_t xCutOff = (w + x < lcd_width) ? 0 : (w + x - lcd_width);
	uint8_t yOffset = (y / 8 < 1) ? 0 : y / 8;

	uint8_t imgheight = (h / 8);
	uint8_t _x = x;
	uint8_t _y = 0;
	uint16_t index = 0;

	if (y >= lcd_height || x >= lcd_width)
		return;

	for (_y = 0; _y < imgheight; _y++)
	{
		for (_x = 0; _x < imgwidth; _x++)
		{

			cbuf = pgm_read_byte(&data[index]);

			if (y % 8 != 0)
			{
				uint16_t tmp = 0;

				uint8_t tmpdat = cbuf;
				tmp = (tmpdat << (y % 8));

				active_buffer[((_y + yOffset) * lcd_width) + _x + x] |= (tmp
						& 255);
				if (_y + yOffset + 1 < lcd_height)
					active_buffer[((_y + yOffset + 1) * lcd_width) + _x + x] |=
							((tmp >> 8) & 255);
			}
			else
				active_buffer[((_y + yOffset) * lcd_width) + _x + x] |= cbuf;
			index++;
		}
		index += xCutOff;
	}
}

void lcd_display::DrawRectangle(int8_t x1, int8_t y1, int8_t x2, int8_t y2,
		uint8_t color = 1, uint8_t fill = 0)
{
	int8_t xdir;
	int8_t xref = (x1 <= x2) ? x2 : x1;
	int8_t ydir = (y1 <= y2) ? y2 : y1;
	int8_t yref = (y1 <= y2) ? y1 : y2;

	for (; ydir >= yref; ydir--)
	{
		PutPixel(x1, ydir, color);
		PutPixel(x2, ydir, color);
		if (fill == 0 && (ydir != y1 && ydir != y2))
			continue;
		for (xdir = (x1 <= x2) ? (x1 + 1) : (x2 + 1); xdir <= (xref - 1);
				xdir++)
		{
			PutPixel(xdir, ydir, color);
		}
	}
}

/**
 * Invert parts of the display. x1 can be greater or lower than x2. Also y1/y2.
 * It will invert from x1 to x2 (including x1 and x2). Also y1/y2 included.
 *
 * @param x1 the left (or right) position
 * @param y1 the top (or bottom) position
 * @param x2 the opposite side to x1
 * @param y2 the opposite side to y1
 */
void lcd_display::Invert(int8_t x1, int8_t y1, int8_t x2, int8_t y2)
{
	int8_t xdir;
	int8_t xref = (x1 <= x2) ? x2 : x1;
	int8_t ydir = (y1 <= y2) ? y2 : y1;
	int8_t yref = (y1 <= y2) ? y1 : y2;

	for (; ydir >= yref; ydir--)
	{
		InvertPixel(x1, ydir);
		InvertPixel(x2, ydir);
		for (xdir = (x1 <= x2) ? (x1 + 1) : (x2 + 1); xdir <= (xref - 1);
				xdir++)
		{
			InvertPixel(xdir, ydir);
		}
	}
}

void lcd_display::InvertPart(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	uint8_t y, x;

	for (y = y1; y <= y2; y++)
	{
		for (x = x1; x < x2; x++)
		{
			active_buffer[lcd_width * y + x] ^= 0xFF;
		}
	}
}

void lcd_display::DrawArc(uint8_t cx, uint8_t cy, uint8_t radius, int16_t start,
		int16_t end)
{
	int16_t angle = 0;
	int8_t x, y;

	for (angle = start; angle <= end; angle++)
	{
		x = radius * get_sin(angle);
		y = radius * get_sin(angle + 180);
		PutPixel(cx + x, cy + y, 1);
	}
}

void lcd_display::DrawTriangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2,
		uint8_t x3, uint8_t y3, uint8_t color)
{
	DrawLine(x1, y1, x2, y2, color);
	DrawLine(x1, y1, x3, y3, color);
	DrawLine(x2, y2, x3, y3, color);
}

void lcd_display::DrawCircle(uint8_t cx, uint8_t cy, uint8_t radius, uint8_t color)
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

void lcd_display::plot8points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y,
		uint8_t color)
{
	plot4points(cx, cy, x, y, color);
	if (x != y)
		plot4points(cx, cy, y, x, color);
}

void lcd_display::plot4points(uint8_t cx, uint8_t cy, uint8_t x, uint8_t y,
		uint8_t color)
{
	PutPixel(cx + x, cy + y, color);
	if (x != 0)
		PutPixel(cx - x, cy + y, color);
	if (y != 0)
		PutPixel(cx + x, cy - y, color);
	if (x != 0 && y != 0)
		PutPixel(cx - x, cy - y, color);
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
		for (uint8_t j = 0; j < 6; j++)
		{
			SetRowCol(5 - j, 0);

			GpioWrite(LCD_DC, HIGH);
			this->spi->SetSlave(LCD_CE);
			for (uint8_t a = 0; a < lcd_width; a++)
			{
				this->spi->SendRaw(
						fast_flip(
								active_buffer[lcd_width - 1 - a
										+ (j * lcd_width)]));
			}
			this->spi->UnsetSlave();
		}
	}
	else
	{
		for (uint8_t j = 0; j < 6; j++)
		{
			SetRowCol(j, 0);

			GpioWrite(LCD_DC, HIGH);
			this->spi->SetSlave(LCD_CE);
			for (uint8_t a = 0; a < lcd_width; a++)
			{
				this->spi->SendRaw(active_buffer[a + (j * lcd_width)]);
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

	for (uint8_t j = 0; j < 6; j++)
	{
		uint16_t index = j * lcd_width + col_x;
		uint8_t cnt = 0;

		for (uint8_t a = start_x; a < end_x; a++)
		{
			this->layers[dst][index + cnt] = this->active_buffer[a
					+ (j * lcd_width)];
			cnt++;
		}
	}
}

void lcd_display::CopyToLayerXPart(uint8_t dst, int8_t x, uint8_t row1,
		uint8_t row2)
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

	for (uint8_t j = row1; j < row2; j++)
	{
		uint16_t index = j * lcd_width + col_x;
		uint8_t cnt = 0;

		for (uint8_t a = start_x; a < end_x; a++)
		{
			this->layers[dst][index + cnt] = this->active_buffer[a
					+ (j * lcd_width)];
			cnt++;
		}
	}
}

void lcd_display::CopyToLayer(uint8_t dst)
{
	memcpy(this->layers[dst], this->active_buffer,
			(lcd_height / 8) * lcd_width);
}

uint8_t * lcd_display::GetActiveLayerPtr()
{
	return this->active_buffer;
}

void lcd_display::CopyToLayerPart(uint8_t dst, uint8_t row1, uint8_t col1,
		uint8_t row2, uint8_t col2)
{
	for (uint8_t j = row1; j < row2; j++)
	{
		uint16_t start_i = j * lcd_width;

		for (uint8_t a = col1; a < col2; a++)
		{
			uint16_t index = start_i + a;
			this->layers[dst][index] = this->active_buffer[index];
		}
	}
}

void lcd_display::ClearBuffer(void)
{
	unsigned char i, k;
	for (k = 0; k < 6; k++)
	{
		for (i = 0; i < lcd_width; i++)     //clear all COL
		{
			active_buffer[i + (k * lcd_width)] = 0;
		}
	}
}

void lcd_display::ClearPart(uint8_t row1, uint8_t col1, uint8_t row2,
		uint8_t col2)
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

/**
 * This table holds sinus values for 0-90 degrees.
 * The value is multiplied by 255 to use a uint8_t * for storage.
 */
const uint8_t PROGMEM sin_table[91] = {
		0, 4, 8, 13, 17, 22, 26, 31, 35, 39, 44, 48, 53, 57, 61, 66, 70,
		74, 78, 83, 87, 91, 95, 99, 103, 107, 111, 115, 119, 123, 127,
		131, 135, 138, 142, 146, 149, 153, 157, 160, 163, 167, 170, 173,
		177, 180, 183, 186, 189, 192, 195, 198, 200, 203, 206, 208, 211,
		213, 216, 218, 220, 223, 225, 227, 229, 231, 232, 234, 236, 238,
		239, 241, 242, 243, 245, 246, 247, 248, 249, 250, 251,
		251, 252, 253, 253, 254, 254, 254, 254, 254, 255 };

float lcd_display::get_sin(uint16_t angle)
{
	angle = angle % 360;
	int16_t value;

	if (angle < 90)
		value = pgm_read_byte(&sin_table[angle]);
	else if (angle < 180)
		value = pgm_read_byte(&sin_table[90 - (angle - 90)]);
	else if (angle < 270)
		value = -pgm_read_byte(&sin_table[angle - 180]);
	else
		value = -pgm_read_byte(&sin_table[90 - (angle - 270)]);

	return (float)value / 255.0;
}

float lcd_display::get_cos(uint16_t angle)
{
	angle += 270;
	return this->get_sin(angle);
}
