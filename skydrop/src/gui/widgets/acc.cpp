#include "acc.h"


#define LPF_beta 		0.05		//low pass filter setting
#define max_hold_time	50	//0.5s	//hold time of last peak value ( 1 = 10ms )


struct gui_widget_accel_t
{
	float old_acc;
	uint8_t hold_time;
};
gui_widget_accel_t gui_widget_accel;


float gui_accel_filter(float new_accel)
{
	if(new_accel >= gui_widget_accel.old_acc)
	{
		gui_widget_accel.old_acc = new_accel;
		gui_widget_accel.hold_time = 0;
		return gui_widget_accel.old_acc;
	}
	if((new_accel < gui_widget_accel.old_acc) and ( gui_widget_accel.hold_time < max_hold_time ) )
	{
		gui_widget_accel.hold_time += 1;
		return gui_widget_accel.old_acc;
	}
	else
	{
		gui_widget_accel.old_acc = (gui_widget_accel.old_acc - (LPF_beta * (gui_widget_accel.old_acc - new_accel)));
		return gui_widget_accel.old_acc;
	}
}


void widget_acc_total_draw(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t flags)
{
	uint8_t lh = widget_label_P(PSTR("TotAcc"), x, y);

	float val = fc.acc.total_gui_filtered;

	char text[10];
	sprintf_P(text, PSTR("%0.1f"), val);

	widget_value_int(text, x, y + lh, w, h - lh);
}


register_widget1(w_acc_tot, "Accelerometer", widget_acc_total_draw);
