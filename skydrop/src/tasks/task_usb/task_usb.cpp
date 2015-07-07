#include "task_usb.h"
#include "../../gui/gui.h"

SleepLock usb_lock;
extern Usart sd_spi_usart;

void task_usb_init()
{

	USB_PWR_ON;
	SD_SPI_PWR_ON;
	SD_EN_ON;


	DEBUG("This is USB task\n");

	usb_lock.Lock();

	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC2MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC2MHZ, DFLL_REF_INT_RC32KHZ, 2000000);
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_USB);

	DEBUG("SD card init in RAW mode ... ");
	if (SDCardManager_Init())
		DEBUG("OK\n");
	else
		DEBUG("Error\n");

	DEBUG("USB init\n");

	USB_Init();

	//init gui
	gui_init();
	gui_switch_task(GUI_USB);
}


void task_usb_stop()
{
	usb_lock.Unlock();

	led_set(0, 0, 0);

	gui_stop();

	sd_spi_usart.Stop();

	USB_PWR_OFF;
	SD_SPI_PWR_OFF;
	SD_EN_OFF;

}


void task_usb_loop()
{
	gui_loop();

	for (uint8_t i=0; i < 128; i++)
	{
		MassStorage_Loop();
		wdt_reset();
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
