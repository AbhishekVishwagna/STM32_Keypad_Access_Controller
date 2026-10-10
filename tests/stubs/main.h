/* Minimal stand-in for CubeMX's main.h + HAL, only so CI can compile-check app.c and actuators.c. */
#ifndef STUB_MAIN_H
#define STUB_MAIN_H

#include <stdint.h>

typedef struct { uint32_t unused; } GPIO_TypeDef;
typedef struct { void *Instance; } UART_HandleTypeDef;
typedef struct { void *Instance; } TIM_HandleTypeDef;
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1 } GPIO_PinState;
typedef enum { HAL_OK = 0, HAL_ERROR = 1 } HAL_StatusTypeDef;

extern GPIO_TypeDef stub_gpio;
#define GPIOA (&stub_gpio)
#define GPIOB (&stub_gpio)

#define TIM_CHANNEL_1 0u

uint32_t HAL_GetTick(void);
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin);
HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *h, uint8_t *data, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef *h, uint8_t *data, uint16_t len);
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t channel);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
#define __HAL_TIM_SET_COMPARE(h, ch, v) ((void)(h), (void)(ch), (void)(v))

#define KP_R0_Pin ((uint16_t)0x0001)
#define KP_R0_GPIO_Port GPIOA
#define KP_R1_Pin ((uint16_t)0x0002)
#define KP_R1_GPIO_Port GPIOA
#define KP_R2_Pin ((uint16_t)0x0004)
#define KP_R2_GPIO_Port GPIOA
#define KP_R3_Pin ((uint16_t)0x0008)
#define KP_R3_GPIO_Port GPIOA
#define KP_C0_Pin ((uint16_t)0x0010)
#define KP_C0_GPIO_Port GPIOB
#define KP_C1_Pin ((uint16_t)0x0020)
#define KP_C1_GPIO_Port GPIOB
#define KP_C2_Pin ((uint16_t)0x0040)
#define KP_C2_GPIO_Port GPIOB
#define LED_A_Pin ((uint16_t)0x0080)
#define LED_A_GPIO_Port GPIOB
#define LED_B_Pin ((uint16_t)0x0100)
#define LED_B_GPIO_Port GPIOB
#define VIB_Pin ((uint16_t)0x0200)
#define VIB_GPIO_Port GPIOB

#endif
