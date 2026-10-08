#ifndef APP_H
#define APP_H

void app_init(void);       /* once, after CubeMX peripheral init */
void app_poll(void);       /* every iteration of the main loop */
void app_tick_1ms(void);   /* from SysTick_Handler (after HAL_IncTick) */

#endif
