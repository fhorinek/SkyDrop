#include "task_active.h"
#include "../gui/gui.h"
#include "../fc/fc.h"

void task_active_init()
{
	DEBUG(" *** THIS IS TASK ACTIVE ***\n");

	gui_init();
	fc_init();
	//bt_init();
}

void task_active_stop()
{
	gui_stop();
}

void task_active_loop()
{

	fc_step();

//	GPIO2_HIGH;
	gui_loop();
//	GPIO2_LOW;

	bt_step();


}

void task_active_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{

	case(TASK_IRQ_BT):
		DEBUG("BT IRQ %d %d\n", buff[0], buff[1]);
		if (*buff == BT_EVENT_INCOMING)
			bt_link.AcceptConnection();
	break;

	default:
		gui_irqh(type, buff);
	}
}
