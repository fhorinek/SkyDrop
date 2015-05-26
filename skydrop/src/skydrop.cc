#include "skydrop.h"

void Setup()
{
	debug_level = 2;

	//set clock to max for init 32MHz
	ClockSetSource(x32MHz);
	//disable 2MHZ osc
	OSC.CTRL = 0b00000010;

	//debug pins
	IO0_INIT;
	IO0_HIGH;

	IO1_INIT;

	//save power
	turnoff_subsystems();

	EnableInterrupts();

	//init basic peripherals
	led_init();
	uart_init_buffers();
	uart_init();
	time_init();
	buzzer_init();
	battery_init();
	buttons_init();

	//basic power control
	MEMS_POWER_INIT;
	I2C_POWER_INIT;
	SD_EN_INIT;

	_delay_ms(100);
}

int free_ram_at_start;

void Post()
{
	//buzzer_beep(_1sec, 0, 0, 0, 0);

	DEBUG("\n *** POST *** \n");

	//Print reset reason
	DEBUG("Reset reason ... ");

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
	char time_tmp[16];
	time_str(time_tmp, time_get_actual());
	DEBUG("Time is ... %s\n", time_tmp);

	DEBUG("Free RAM at start ... %d\n", free_ram_at_start);
	test_memory();

	DEBUG("\n");
}

int main()
{
	free_ram_at_start = freeRam();
	Setup();

	Post();

	DEBUG(" *** STARTING TASKS ***\n");

	task_init();

	uint32_t counter_next = 0;

	while (1)
	{
		//run main task
		task_loop();

		//run system tasks
		task_system_loop();

		//sleep now
		task_sleep();

		if (counter_next < task_get_ms_tick())
		{
			//print_cpu_usage();
			counter_next = task_get_ms_tick() + 5000;
		}
	}
}
