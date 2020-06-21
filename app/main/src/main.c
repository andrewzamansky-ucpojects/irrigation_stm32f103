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


extern struct dev_desc_t * adc_humidity_dev;
extern struct dev_desc_t * adc_battery_dev;
extern struct dev_desc_t * motor_control_pin_dev;
extern struct dev_desc_t * blink_led_dev;
extern struct dev_desc_t * pwr_dev;
extern struct dev_desc_t * rtc_dev;

// alpha = 0.1  = 1/10 ;   (1 - alpha) = 0.9
#define SMOOTH_ALPHA_NUMERATOR    1
#define SMOOTH_ALPHA_DENOMERATOR  10
#define SMOOTH_ALPHA_DENOMERATOR_MINUS_NUMERATOR  \
				(SMOOTH_ALPHA_DENOMERATOR - SMOOTH_ALPHA_NUMERATOR)

#define NUM_OF_MEASUREMENTS   100 // 10
#define DELAY_BETWEEN_ADC_SAMPLES_mSEC   2
static uint8_t rtc_calibration_done = 0;

#define MOISTURE_LOW_THRESHOLD_mV  2100

// measures on voltage divider resistor with ~1/10 ratio so threshold = 4750mV
#define BATTERY_THRESHOLD_mV  475

#define STATUS_OK_DELAY 1000
#define STATUS_LOW_BATTERY_DELAY 50

#define STANDBY_NORMAL_TIME  (12 * 60 * 60 * 1000) // 12hours

// small standby time to increase chances that blinks or beeps will
// be noticed by human
#define STANDBY_LOW_BATTERY_TIME  (30 * 1000) // 30 seconds

/**
 * init_hw()
 *
 * return:
 */
static void init_hw(void)
{
	struct dev_desc_t * dev;

	DEV_IOCTL_0_PARAMS(blink_led_dev, IOCTL_DEVICE_START);

	dev = DEV_OPEN("semihosting_dev");
	if (NULL != dev)
	{
		DEV_IOCTL_0_PARAMS(dev, IOCTL_DEVICE_START);
		PRINTF_API_AddDebugOutput(dev);
	}

	DEV_IOCTL_0_PARAMS(adc_humidity_dev, IOCTL_DEVICE_START);
	DEV_IOCTL_0_PARAMS(adc_battery_dev, IOCTL_DEVICE_START);
	DEV_IOCTL_0_PARAMS(motor_control_pin_dev, IOCTL_DEVICE_START);
	DEV_IOCTL_0_PARAMS(motor_control_pin_dev, IOCTL_GPIO_PIN_CLEAR );
}


/**
 * make_measurements()
 *
 * return:
 */
static void make_measurements(uint32_t *humidity, uint32_t *battery_level)
{
	uint32_t cnt;
	uint32_t adc_humidity_val;
	uint32_t adc_battery_val;
	uint32_t battery_avg_val;
	uint32_t smoothed_adc_humidity_val;

	smoothed_adc_humidity_val = 500;// not relevant for avaraging
	battery_avg_val = 0;

	for (cnt = 0; cnt < NUM_OF_MEASUREMENTS; cnt++)
	{
		DEV_IOCTL_1_PARAMS(adc_battery_dev,
				IOCTL_ADC_GET_CURRENT_VALUE_mV, &adc_battery_val);
		PRINTF_DBG("%05d  bat = %d.%03dV; ",
				cnt, adc_battery_val / 1000, adc_battery_val % 1000);

		battery_avg_val *= (cnt - 1);
		battery_avg_val += adc_battery_val;
		battery_avg_val /= cnt;
		PRINTF_DBG("bat_avg = %d.%03dV\r\n",
				battery_avg_val / 1000, battery_avg_val % 1000);

		DEV_IOCTL_1_PARAMS(adc_humidity_dev,
				IOCTL_ADC_GET_CURRENT_VALUE_mV, &adc_humidity_val);
		PRINTF_DBG("%d.%03dV; ",
				adc_humidity_val / 1000, adc_humidity_val % 1000);
#if 0
		// smoothed_adc_humidity_val =
		// (1 - alpha) * smoothed_adc_humidity_val + alpha * adc_humidity_val
		adc_humidity_val *= SMOOTH_ALPHA_NUMERATOR;
		smoothed_adc_humidity_val *= SMOOTH_ALPHA_DENOMERATOR_MINUS_NUMERATOR;
		smoothed_adc_humidity_val += adc_humidity_val;
		smoothed_adc_humidity_val /= SMOOTH_ALPHA_DENOMERATOR;
#else
		smoothed_adc_humidity_val *= (cnt - 1);
		smoothed_adc_humidity_val += adc_humidity_val;
		smoothed_adc_humidity_val /= cnt;
#endif
		PRINTF_DBG("smoothed %d.%03dV\r\n",
			smoothed_adc_humidity_val / 1000, smoothed_adc_humidity_val % 1000);


		while (PRINTF_API_print_from_debug_buffer(64));

		os_delay_ms(DELAY_BETWEEN_ADC_SAMPLES_mSEC);

	}
	*humidity = smoothed_adc_humidity_val;
	*battery_level = battery_avg_val;
}


static void blink_status()
{
	uint8_t blink_state;
	uint8_t blink_delay_ms;
	uint8_t i;

	blink_state = 0;
	blink_delay_ms = STATUS_LOW_BATTERY_DELAY;
	for (i = 0; i < 10; i++)
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
	}

}


/**
 * measurements_thread_func()
 *
 * return:
 */
static void measurements_thread_func(void * aHandle)
{
	uint32_t wakeup_val_mSec;
	uint32_t humidity;
	uint32_t battery_level;

	init_hw();
	make_measurements(&humidity, &battery_level);

	if (battery_level < BATTERY_THRESHOLD_mV)
	{
		blink_status();
		wakeup_val_mSec = STANDBY_LOW_BATTERY_TIME;
	}
	else
	{
		if (MOISTURE_LOW_THRESHOLD_mV > humidity)
		{
			DEV_IOCTL_0_PARAMS(motor_control_pin_dev, IOCTL_GPIO_PIN_SET );
		}
		os_delay_ms(3000);
		DEV_IOCTL_0_PARAMS(motor_control_pin_dev, IOCTL_GPIO_PIN_CLEAR );
		wakeup_val_mSec = STANDBY_NORMAL_TIME;
	}

	while (0 == rtc_calibration_done)
	{
		os_delay_ms(50);
	}

	os_stack_test(); //requires PRINTF_DBG
	while (PRINTF_API_print_from_debug_buffer(64));

	//while(1){os_delay_ms(1000);} // to remove, for dbg only

	DEV_IOCTL_1_PARAMS(rtc_dev, IOCTL_RTC_SET_WAKEUP_mSec, &wakeup_val_mSec);
	DEV_IOCTL_0_PARAMS(pwr_dev, IOCTL_POWER_MANAGEMENT_ENTER_HIBERNATION);
	// should not reach here
	while(1)
	{
		os_delay_ms(1000);
	}

}


/**
 * rtc_calibration_thread_func()
 *
 * do RTC clock calibration for 100ms while other tasks perform measurments
 */
static void rtc_calibration_thread_func(void * aHandle)
{

	DEV_IOCTL_0_PARAMS(pwr_dev, IOCTL_DEVICE_START);//should be before RTC
	DEV_IOCTL_0_PARAMS(rtc_dev, IOCTL_DEVICE_START);
	DEV_IOCTL_0_PARAMS(rtc_dev, IOCTL_RTC_CALIBRATE);

	rtc_calibration_done = 1;
	while(1)
	{
		os_delay_ms(1000);
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

	os_create_task("test_thread", measurements_thread_func, NULL,
				MEASUREMENTS_THREAD_STACK_SIZE_BYTES,
				MEASUREMENTS_THREAD_PRIORITY);
	os_create_task("test_thread", rtc_calibration_thread_func, NULL,
		RTC_CALIBRATION_THREAD_STACK_SIZE_BYTES,
		RTC_CALIBRATION_THREAD_PRIORITY);

	os_start();

	while(1);
error :
	while(1);
}


