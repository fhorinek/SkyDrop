#ifndef N5110_DISP_H_
#define N5110_DISP_H_

#include "../common.h"
#include "../data/fonts.h"
#include <math.h>

#define DISP_COLOR_WHITE	1
#define DISP_LAYERS			2

#define lcd_width		84
#define lcd_height		48
#define lcb_layer_size	((lcd_height / 8) * lcd_width)

class lcd_display{
private:
	Spi * spi;
	uint8_t * active_buffer;

	uint8_t layers[DISP_LAYERS][lcb_layer_size];

	void sendcommand(unsigned char cmd);
	void SendChar(unsigned char data);
	void plot8points(uint8_t cx,uint8_t cy,uint8_t x,uint8_t y,uint8_t color);
	void plot4points(uint8_t cx,uint8_t cy,uint8_t x,uint8_t y,uint8_t color);

	void CreateSinTable();

	float sin_table[91];

	uint8_t text_x;
	uint16_t text_y;

	const uint8_t * font_data;
	uint8_t font_spacing;
	uint8_t font_lines;
	uint8_t font_height;
	uint8_t font_A_height;

	uint8_t font_begin;
	uint8_t font_end;
	uint8_t font_adr_start;

	bool flip;
public:
	float get_cos(uint16_t angle);
	float get_sin(uint16_t angle);

	void Init(Spi * spi);
	void Stop();

	void SetContrast(uint8_t val); //0-127
	void SetInvert(uint8_t invert);
	void SetFlip(bool flip);

	void ClearBuffer();
	void PutPixel(uint8_t x ,uint16_t  y ,uint8_t color);
	void DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t color);
	void DrawCircle(uint8_t cx,uint8_t cy,uint8_t radius,uint8_t color);
	void DrawRectangle(int8_t x1,int8_t y1,int8_t x2,int8_t y2,uint8_t color,uint8_t fill);
	void DrawTriangle(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t x3,uint8_t y3,uint8_t color);
	void DrawArc(uint8_t cx,uint8_t cy,uint8_t radius,int16_t start,int16_t end);
	void DrawImage(const uint8_t *data,uint8_t x,uint8_t y);
	void Write(uint8_t ascii);
	void SetRowCol(unsigned char row,unsigned char col);
	void Draw();
	void GotoXY(uint8_t x, uint16_t y);
	void InvertPixel(uint8_t x ,uint8_t  y);
	void Invert(int8_t x1,int8_t y1,int8_t x2,int8_t y2);
	void LoadFont(const uint8_t * font);

	void ClearPart(uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2);
	void InvertPart(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2);

	uint8_t GetTextWidth(char * text);
	uint8_t GetTextWidthN(char * text, uint8_t n);
	uint8_t GetTextHeight();
	uint8_t GetAHeight();

	void CopyToLayerX(uint8_t dst, int8_t x);
	void CopyToLayerXPart(uint8_t dst, int8_t x, uint8_t row1, uint8_t row2);
	void CopyToLayerPart(uint8_t dst, uint8_t row1, uint8_t col1, uint8_t row2, uint8_t col2);
	void CopyToLayer(uint8_t dst);

	void SetDrawLayer(uint8_t layer);

	uint8_t * GetActiveLayerPtr();
};

#endif /* OLEDDISPLAY_H_ */
