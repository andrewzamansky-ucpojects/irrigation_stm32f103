/*
 *
 *   file  :  main.cpp
 *
 */



/***************   includes    *******************/

#include "_project_typedefs.h"
#include "_project_defines.h"
#include "_project_func_declarations.h"
#include "_project_tasks_defines.h"

#include "dev_management_api.h"

#include "os_wrapper.h"
#include "adc_api.h"

#define DEBUG
#include "PRINTF_api.h"

/***************   defines    *******************/


/***************   typedefs    *******************/


/**********   external variables    **************/

/***********   loacal variables    **************/

extern struct dev_desc_t * adc_dev;
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

	dev = DEV_OPEN("semihosting_dev");
	if (NULL != dev)
	{
		DEV_IOCTL_0_PARAMS(dev, IOCTL_DEVICE_START);
		PRINTF_API_AddDebugOutput(dev);
	}

	DEV_IOCTL_0_PARAMS(adc_dev, IOCTL_DEVICE_START);

	cnt = 0;
	while (1)
	{
		cnt++;
		DEV_IOCTL_1_PARAMS(adc_dev, IOCTL_ADC_GET_CURRENT_VALUE_mV, &adc_val);
		PRINTF_DBG("%05d  %d.%03dV\r\n", cnt, adc_val / 1000, adc_val % 1000);

		while (PRINTF_API_print_from_debug_buffer(64));

		os_delay_ms(1000);

		//os_stack_test(); //requires PRINTF_DBG
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

	os_create_task("test_thread" , test_thread_func, NULL,
				TEST_THREAD_STACK_SIZE_BYTES, TEST_THREAD_PRIORITY);

	os_start();

	while(1);
error :
	while(1);
}


