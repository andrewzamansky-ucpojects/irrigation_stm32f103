#include "_project.h"

#include "dev_management_api.h"
#include "dt_pin_control_stm32f10x.h"


/***********************************/
/********** soc_clock_control_dev ********/
#define DT_DEV_NAME                        soc_clock_control_dev
#define DT_DEV_MODULE                      clock_control_stm32f10x

#define CLOCK_CONTROL_STM32f10X_DT_HIRC_RATE           8000000
#define CLOCK_CONTROL_STM32f10X_DT_LIRC_RATE           40000
//#define CLOCK_CONTROL_STM32f10X_DT_XTAL_RATE           8000000
#define CLOCK_CONTROL_STM32f10X_DT_SYSCLK_SRC_CLK_DEV    stm32f10x_hsirc_clk_dev//stm32f10x_hsirc_clk_dev//stm32f10x_xtal_clk_dev
//#define CLOCK_CONTROL_STM32f10X_DT_PLL_SRC_CLK_DEV     stm32f10x_hsirc_clk_dev//stm32f10x_xtal_clk_dev
//#define CLOCK_CONTROL_STM32f10X_DT_PLL_RATE        72000000
#define CLOCK_CONTROL_STM32f10X_DT_HCLK_RATE       8000000//72000000
#define CLOCK_CONTROL_STM32f10X_DT_APB1_RATE          8000000
#define CLOCK_CONTROL_STM32f10X_DT_APB2_RATE          8000000

#include ADD_CURRENT_DEV




/***********************************/
/********** systick_dev ********/
#define DT_DEV_NAME                        systick_dev
#define DT_DEV_MODULE                      cortexM_systick

#define CORTEXM_SYSTICK_DT_CLOCK_PDEV      stm32f10x_systick_clk_dev
#define CORTEXM_SYSTICK_DT_INITIAL_RATE    OS_TICK_IN_MICRO_SEC
#define CORTEXM_SYSTICK_DT_MODE            TIMER_API_PERIODIC_MODE

#include ADD_CURRENT_DEV




/***********************************/
/********** adc_humidity_dev ********/
#define DT_DEV_NAME                        adc_humidity_dev
#define DT_DEV_MODULE                      adc_stm32f10x

#define ADC_STM32F10X_DT_CLOCK_RATE      4000000
#define ADC_STM32F10X_DT_INPUT_PIN     PIN_CONTROL_DT_STM32F10X_PIN_A00_ADC

#include ADD_CURRENT_DEV



/***********************************/
/********** adc_battery_dev ********/
#define DT_DEV_NAME                        adc_battery_dev
#define DT_DEV_MODULE                      adc_stm32f10x

#define ADC_STM32F10X_DT_CLOCK_RATE      4000000
#define ADC_STM32F10X_DT_INPUT_PIN     PIN_CONTROL_DT_STM32F10X_PIN_A02_ADC

#include ADD_CURRENT_DEV




/***********************************/
/********** rtc_dev ********/
#define DT_DEV_NAME                        rtc_dev
#define DT_DEV_MODULE                      rtc_stm32f10x

#define RTC_STM32F10X_DT_SRC_CLOCK_DEV      stm32f10x_lsirc_clk_dev
#define RTC_STM32F10X_DT_COUNTER_RATE_HZ    1//625

#include ADD_CURRENT_DEV




/***********************************/
/********** pwr_dev ********/
#define DT_DEV_NAME                        pwr_dev
#define DT_DEV_MODULE                      pwr_stm32f10x

#include ADD_CURRENT_DEV




/***********************************/
/********** semihosting_dev ********/
#define DT_DEV_NAME                      semihosting_dev
#define DT_DEV_MODULE                    semihosting

#include ADD_CURRENT_DEV


/***********************************/
/********** motor_control_pin_dev  ********/
#define DT_DEV_NAME                      motor_control_pin_dev
#define DT_DEV_MODULE                    gpio_stm32f10x

#define GPIO_STM32F10X_DT_PORT         GPIO_STM32F10X_API_PORT_A
#define GPIO_STM32F10X_DT_PINS             {1}
#define GPIO_STM32F10X_DT_PINS_IDLE_STATE  {0}
#define GPIO_STM32F10X_DT_MODE         GPIO_STM32F10X_API_MODE_OUT_PP

#include ADD_CURRENT_DEV


/***********************************/
/********** blink_led_dev  ********/
#define DT_DEV_NAME                      blink_led_dev
#define DT_DEV_MODULE                    gpio_stm32f10x

#define GPIO_STM32F10X_DT_PORT         GPIO_STM32F10X_API_PORT_C
#define GPIO_STM32F10X_DT_PINS             {13}
#define GPIO_STM32F10X_DT_PINS_IDLE_STATE  {0}
#define GPIO_STM32F10X_DT_MODE         GPIO_STM32F10X_API_MODE_OUT_PP

#include ADD_CURRENT_DEV


/***********************************/
/********** pwm_dev  ********/
#define DT_DEV_NAME                      pwm_dev
#define DT_DEV_MODULE                    pwm_stm32f10x

#define PWM_STM32F10X_DT_OUTPUT_PIN     PIN_CONTROL_DT_STM32F10X_PIN_A08_PWM

#include ADD_CURRENT_DEV


