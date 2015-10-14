#include "skydrop.h"

void Setup()
{
	debug_level = 2;

	//set clock to max for init 32MHz
	ClockSetSource(x32MHz);
	//disable 2MHZ osc
	OSC.CTRL = 0b00000010;

	//save power
	turnoff_subsystems();

	EnableInterrupts();

	//load device id
	GetID();

	//init basic peripherals
	led_init();
	uart_init_buffers();
	uart_init();
	time_init();
	buzzer_init();
	battery_init();
	buttons_init();

	//basic power control
	mems_power_init();
	io_init();
	SD_EN_INIT;

	//load configuration
	cfg_load();

	_delay_ms(100);
}

int free_ram_at_start;
uint8_t system_rst;

void Post()
{
	DEBUG("\n *** POST *** \n");

	//Print reset reason
	DEBUG("Reset reason ... ");

	system_rst = RST.STATUS;

	if (RST.STATUS & 0b00100000)
		DEBUG("Software ");
	else
	if (RST.STATUS & 0b00010000)
		DEBUG("Programming ");
	else
	if (RST.STATUS & 0b00001000)
		DEBUG("Watchdog ");
	else
	if (RST.STATUS & 0b00000100)
		DEBUG("Brownout ");
	else
	if (RST.STATUS & 0b00000010)
		DEBUG("External ");
	else
	if (RST.STATUS & 0b00000001)
		DEBUG("Power On ");
	else
		DEBUG("Unknown: %02X", RST.STATUS);

	RST.STATUS = 0b00111111;
	DEBUG("\n");

	//App name
	print_fw_info();

	//Print actual time
	DEBUG("Time is ... \n");
	print_datetime(time_get_actual());

	DEBUG("Free RAM at start ... %d\n", free_ram_at_start);
	test_memory();

	char id[22];
	GetID_str(id);
	DEBUG("Device serial number ... %s\n", id);

	DEBUG("\n");
}

extern uint8_t actual_task;
extern uint8_t task_sleep_lock;

int main()
{
	free_ram_at_start = freeRam();
	Setup();

	Post();

	DEBUG(" *** STARTING TASKS ***\n");

	task_init();

	while (1)
	{
		//run main task
		task_loop();

		//run system tasks
		task_system_loop();

		//sleep now
		task_sleep();
	}
}
