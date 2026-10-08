#include "actuators.h"
#include "app_pins.h"

extern TIM_HandleTypeDef APP_SERVO_TIM;

static volatile uint16_t g_vib_left_ms;

void actuators_init(void)
{
    led_set(LED_OFF);
    HAL_GPIO_WritePin(VIB_GPIO_Port, VIB_Pin, GPIO_PIN_RESET);
    (void)HAL_TIM_PWM_Start(&APP_SERVO_TIM, APP_SERVO_CHANNEL);
}

void led_set(led_color_t color)
{
    HAL_GPIO_WritePin(LED_A_GPIO_Port, LED_A_Pin, (color == LED_RED)   ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, (color == LED_GREEN) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void servo_set_us(uint16_t pulse_us)
{
    __HAL_TIM_SET_COMPARE(&APP_SERVO_TIM, APP_SERVO_CHANNEL, pulse_us);
}

void vib_pulse(uint16_t ms)
{
    g_vib_left_ms = ms;
    HAL_GPIO_WritePin(VIB_GPIO_Port, VIB_Pin, GPIO_PIN_SET);
}

void actuators_tick_1ms(void)
{
    if (g_vib_left_ms != 0u) {
        g_vib_left_ms--;
        if (g_vib_left_ms == 0u) {
            HAL_GPIO_WritePin(VIB_GPIO_Port, VIB_Pin, GPIO_PIN_RESET);
        }
    }
}
