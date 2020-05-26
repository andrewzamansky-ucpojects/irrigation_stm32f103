/*
 *
 *   file  :  main.cpp
 *
 */
#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"
#include "_project_tasks_defines.h"

#include "dev_management_api.h"

#include "os_wrapper.h"
#include "adc_api.h"
#include "gpio_api.h"
#include "rtc_api.h"
#include "pwr_management_api.h"

#define DEBUG
#include "PRINTF_api.h"


extern struct dev_desc_t * adc_dev;
extern struct dev_desc_t * motor_control_pin_dev;
extern struct dev_desc_t * blink_led_dev;
extern struct dev_desc_t * pwr_dev;
extern struct dev_desc_t * rtc_dev;

// alpha = 0.1  = 1/10 ;   (1 - alpha) = 0.9
#define SMOOTH_ALPHA_NUMERATOR    1
#define SMOOTH_ALPHA_DENOMERATOR  10
#define SMOOTH_ALPHA_DENOMERATOR_MINUS_NUMERATOR  \
				(SMOOTH_ALPHA_DENOMERATOR - SMOOTH_ALPHA_NUMERATOR)

/**
 * test_thread_func()
 *
 * return:
 */
static void test_thread_func(void * aHandle)
{
	uint32_t cnt;
	struct dev_desc_t * dev;
	uint32_t adc_val;
	uint32_t smoothed_adc_val;
	uint32_t wakeup_val_mSec;

	dev = DEV_OPEN("semihosting_dev");
	if (NULL != dev)
	{
		DEV_IOCTL_0_PARAMS(dev, IOCTL_DEVICE_START);
		PRINTF_API_AddDebugOutput(dev);
	}

	DEV_IOCTL_0_PARAMS(adc_dev, IOCTL_DEVICE_START);
	DEV_IOCTL_0_PARAMS(motor_control_pin_dev, IOCTL_DEVICE_START);

	DEV_IOCTL_0_PARAMS(pwr_dev, IOCTL_DEVICE_START);//should be before RTC
	DEV_IOCTL_0_PARAMS(rtc_dev, IOCTL_DEVICE_START);
	DEV_IOCTL_0_PARAMS(rtc_dev, IOCTL_RTC_CALIBRATE);
	cnt = 0;
	smoothed_adc_val = 500;

	wakeup_val_mSec = 10000;
	DEV_IOCTL_1_PARAMS(rtc_dev, IOCTL_RTC_SET_WAKEUP_mSec, &wakeup_val_mSec);

	while (1)
	{
		cnt++;
		if (5 == cnt) // sleep try
		{
			DEV_IOCTL_0_PARAMS(
					pwr_dev, IOCTL_POWER_MANAGEMENT_ENTER_HIBERNATION);
		}

		DEV_IOCTL_1_PARAMS(adc_dev, IOCTL_ADC_GET_CURRENT_VALUE_mV, &adc_val);
		PRINTF_DBG("%05d  %d.%03dV; ", cnt, adc_val / 1000, adc_val % 1000);

		// smoothed_adc_val = (1 - alpha) * smoothed_adc_val + alpha * adc_val
		adc_val *= SMOOTH_ALPHA_NUMERATOR;
		smoothed_adc_val *= SMOOTH_ALPHA_DENOMERATOR_MINUS_NUMERATOR;
		smoothed_adc_val += adc_val;
		smoothed_adc_val /= SMOOTH_ALPHA_DENOMERATOR;
		PRINTF_DBG("smoothed %d.%03dV\r\n",
				smoothed_adc_val / 1000, smoothed_adc_val % 1000);

		if (200 > smoothed_adc_val)
		{
			DEV_IOCTL_0_PARAMS(motor_control_pin_dev, IOCTL_GPIO_PIN_SET );
		}
		else
		{
			DEV_IOCTL_0_PARAMS(motor_control_pin_dev, IOCTL_GPIO_PIN_CLEAR );
		}

		while (PRINTF_API_print_from_debug_buffer(64));

		os_delay_ms(1000);

		os_stack_test(); //requires PRINTF_DBG
	}
}


#define STATUS_OK_DELAY 1000
#define STATUS_LOW_BATTERY_DELAY 100
/**
 * status_report_thread_func()
 *
 * return:
 */
static void status_report_thread_func(void * aHandle)
{
	uint8_t blink_state;
	uint8_t blink_delay_ms;

	DEV_IOCTL_0_PARAMS(blink_led_dev, IOCTL_DEVICE_START);

	blink_state = 0;
	blink_delay_ms = STATUS_LOW_BATTERY_DELAY;
	while (1)
	{
		if (blink_state)
		{
			DEV_IOCTL_0_PARAMS(blink_led_dev, IOCTL_GPIO_PIN_SET );
		}
		else
		{
			DEV_IOCTL_0_PARAMS(blink_led_dev, IOCTL_GPIO_PIN_CLEAR );
		}
		blink_state = 1 - blink_state;


		os_delay_ms(blink_delay_ms);

		os_stack_test(); //requires PRINTF_DBG
	}
}


/*
 * function : main()
 *
 */
int main(void)
{
	struct dev_desc_t * dev;

	dev = DEV_OPEN("soc_clock_control_dev");
	if (NULL == dev) goto error;
	DEV_IOCTL_0_PARAMS(dev , IOCTL_DEVICE_START  );

	dev = DEV_OPEN("systick_dev");
	if (NULL == dev) goto error;
	os_set_tick_timer_dev(dev);
	os_init();

	PRINTF_API_init();

	os_create_task("test_thread", test_thread_func, NULL,
				TEST_THREAD_STACK_SIZE_BYTES, TEST_THREAD_PRIORITY);
	os_create_task("test_thread", status_report_thread_func, NULL,
		STATUS_REPORT_THREAD_STACK_SIZE_BYTES, STATUS_REPORT_THREAD_PRIORITY);

	os_start();

	while(1);
error :
	while(1);
}


