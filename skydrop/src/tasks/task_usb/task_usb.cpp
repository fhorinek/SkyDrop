#include "task_usb.h"
#include "../../gui/gui.h"

SleepLock usb_lock;
extern Usart sd_spi_usart;
extern uint16_t usb_loop_time;

uint8_t task_usb_sd_ready = false;

uint32_t task_usb_gui_timer = 0;
#define TASK_USB_GUI_REFRESH	250ul

void task_usb_init()
{
	SD_EN_OFF;
	_delay_ms(200);

	USB_PWR_ON;
	SD_SPI_PWR_ON;
	SD_EN_ON;

	DEBUG("This is USB task\n");

	usb_lock.Lock();

	cli();
	//Start 2MHz OSC
	assert(XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC2MHZ));
	//Calibrate 2MHz OSC
	assert(XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC2MHZ, DFLL_REF_INT_RC32KHZ, 2000000ul));
	//Multiply 2MHz OSC
	assert(XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000ul, F_CPU));
	//Set PLL as main clock source
	assert(XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL));

	//Stop DFLL
	assert(XMEGACLK_StopDFLL(CLOCK_SRC_INT_RC32MHZ));
	//Stop 32MHz OSC
	XMEGACLK_StopInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	//Start 32MHz OSC
	assert(XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ));
	//Calibrate 32MHz using USB Start Of Frame
	assert(XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB));
	sei();

	DEBUG("SD card init in RAW mode ... ");
	if ((task_usb_sd_ready = SDCardManager_Init()))
	{
		DEBUG("OK\n");

		DEBUG("USB init\n");
		USB_Init();
	}
	else
	{
		DEBUG("Error\n");

		sd_spi_usart.Stop();
		USB_PWR_OFF;
		SD_SPI_PWR_OFF;
		SD_EN_OFF;
		GpioSetDirection(SD_SS, INPUT);
	}

	//init gui
	gui_init();
	gui_switch_task(GUI_USB);
}

#define RCOSC32MA_offset 0x04
#define RCOSC32M_offset 0x03

void task_usb_stop()
{
	USB_Disable();

	cli();
	//Stop 32MHz DFLL
	assert(XMEGACLK_StopDFLL(CLOCK_SRC_INT_RC32MHZ));
	//Stop 32MHz
	XMEGACLK_StopInternalOscillator(CLOCK_SRC_INT_RC32MHZ);

	//Read calibration from signature row
	DFLLRC32M.CALA = SP_ReadCalibrationByte(PROD_SIGNATURES_START + RCOSC32MA_offset);
	DFLLRC32M.CALB = SP_ReadCalibrationByte(PROD_SIGNATURES_START + RCOSC32M_offset);

	//Start 32MHz OSC
	assert(XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ));
	//Set 32MHz as clock source
	assert(XMEGACLK_SetCPUClockSource(CLOCK_SRC_INT_RC32MHZ));

	//Stop PLL
	XMEGACLK_StopPLL();
	assert(XMEGACLK_StopDFLL(CLOCK_SRC_INT_RC2MHZ));
	assert(XMEGACLK_StopInternalOscillator(CLOCK_SRC_INT_RC2MHZ));
	sei();

	usb_lock.Unlock();

	led_set(0, 0, 0);

	gui_stop();

	if (task_usb_sd_ready)
	{
		sd_spi_usart.Stop();

		USB_PWR_OFF;
		SD_SPI_PWR_OFF;
		SD_EN_OFF;
		GpioSetDirection(SD_SS, INPUT);
		_delay_ms(200);
	}

}

void task_usb_loop()
{
	if (task_usb_gui_timer < task_get_ms_tick())
	{
		gui_loop();
	}

	uint32_t start = task_get_ms_tick_once();
	for (uint8_t i=0; i < 32; i++)
	{
		if (task_usb_sd_ready)
			MassStorage_Loop();

		ewdt_reset();
	}
	uint32_t delta = task_get_ms_tick_once() - start;
	usb_loop_time = min(delta / 100, 15);

	if (usb_int_state == USB_NOT_RDY)
		usb_lock.Unlock();
	else
		usb_lock.Lock();
}


void task_usb_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
	case(TASK_IRQ_USB):
		if (*buff == 0)
			task_set(TASK_POWERDOWN);
	break;

	default:
		gui_irqh(type, buff);
	}
}
