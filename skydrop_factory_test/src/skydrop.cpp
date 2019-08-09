#include "skydrop.h"

int free_ram_at_start;
uint8_t system_rst;

void Setup()
{
	//set clock to max for init 32MHz
	ClockSetSource(x32MHz);
	//disable 2MHZ osc
	OSC.CTRL = 0b00000010;

	//get RAM info
	free_ram_at_start = freeRam();

	//get reset reason
	system_rst = RST.STATUS;
	RST.STATUS = 0b00111111;

	//save power - peripherals are turned on on demand by drivers
	turnoff_subsystems();

	EnableInterrupts();

	//load device id
	GetID();

	//init basic peripherals
	led_init();
	buzzer_init();
	buttons_init();

	//basic power control
	mems_power_init();
	io_init();
	SD_EN_INIT;

	//load configuration from EE
	cfg_load();
	uart_init();
	battery_init();


	_delay_ms(100);
}

void Post()
{
	DEBUG("\n *** POST *** \n");

	//Reset reason
	print_reset_reason();

	//App name
	print_fw_info();

	DEBUG("Free RAM at start ... %d\n", free_ram_at_start);
	test_memory();

	char id[23];
	GetID_str(id);
	DEBUG("Device serial number ... %s\n", id);

	DEBUG("Board rev ... %u\n", (hw_revision == HW_REW_1504) ? 1504 : 1406);

	//debug info
	debug_last_dump();
}

int main()
{
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
