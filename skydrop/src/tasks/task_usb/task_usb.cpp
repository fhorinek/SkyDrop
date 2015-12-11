#include "task_usb.h"
#include "../../gui/gui.h"

SleepLock usb_lock;
extern Usart sd_spi_usart;

uint8_t task_usb_sd_ready = false;

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
	assert(XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_RC32KHZ, F_CPU));

	assert(XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC2MHZ));
	assert(XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC2MHZ, DFLL_REF_INT_RC32KHZ, 2000000ul));
	assert(XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000ul, F_USB));
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
	}

	//init gui
	gui_init();
	gui_switch_task(GUI_USB);
}


void task_usb_stop()
{

	cli();
	assert(XMEGACLK_StopDFLL(CLOCK_SRC_INT_RC2MHZ));
	assert(XMEGACLK_StopDFLL(CLOCK_SRC_INT_RC32MHZ));
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
	}

}


uint32_t task_usb_gui_timer = 0;
#define TASK_USB_GUI_REFRESH	2000ul

void task_usb_loop()
{
	if (task_usb_gui_timer < task_get_ms_tick())
	{
		gui_loop();
		task_usb_gui_timer = task_get_ms_tick() + TASK_USB_GUI_REFRESH;
	}

	for (uint8_t i=0; i < 128; i++)
	{
		if (task_usb_sd_ready)
			MassStorage_Loop();

		ewdt_reset();
	}

}


void task_usb_irqh(uint8_t type, uint8_t * buff)
{
	switch (type)
	{
	case(TASK_IRQ_USB):
		if (*buff == 0)
			task_set(TASK_ACTIVE);
	break;

	default:
		gui_irqh(type, buff);
	}
}
