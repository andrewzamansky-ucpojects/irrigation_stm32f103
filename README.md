adc sample of stm32f10x

for using semihost with jlink read "common_public/c/hw_drivers/arch/ARM/semihosting/connect_jlink_with_semihosting.txt"

Debugging:
uncomment following line in main.c(in measurements_thread_func())
before entering hibernation:
//while(1){os_delay_ms(1000);} // to remove, for dbg only

to start JLINK:
for SW thar runs from flash: set BOOT0 and BOOT1 to 1
"C:\Program Files (x86)\SEGGER\JLink\JLink.exe" -device STM32F103C8 -if SWD -speed 4000

to connect to CPU in JLINK cmd line run:
connect

some times board needs to be reset(button) few times before it can connect.


to connect jlink gdb server run:
"C:\Program Files (x86)\SEGGER\JLink\JLinkgdbserver.exe" -device STM32F103C8 -if SWD -speed 4000
