#include "app.h"
#include "app_config.h"
#include "app_pins.h"
#include "access.h"
#include "actuators.h"
#include "event_log.h"
#include "keypad.h"
#include "pin_store.h"
#include "ringbuf.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef APP_UART;

static access_t  g_access;
static keypad_t  g_keypad;
static shell_t   g_shell;
static ringbuf_t g_rx;
static uint8_t   g_rx_storage[64];
static uint8_t   g_rx_byte;
static acc_state_t g_last_state = ACC_LOCKED;

/* ---- UART (blocking transmit keeps this simple; receive is interrupt-driven) ---- */
static void uart_write(const char *s)
{
    (void)HAL_UART_Transmit(&APP_UART, (uint8_t *)s, (uint16_t)strlen(s), 100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &APP_UART) {
        (void)ringbuf_put(&g_rx, g_rx_byte);   /* drop on overflow */
        (void)HAL_UART_Receive_IT(&APP_UART, &g_rx_byte, 1);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == &APP_UART) {
        (void)HAL_UART_Receive_IT(&APP_UART, &g_rx_byte, 1);   /* recover from overrun/noise */
    }
}

/* ---- Keypad hardware hooks ---- */
static GPIO_TypeDef *const COL_PORT[KEYPAD_COLS] = {KP_C0_GPIO_Port, KP_C1_GPIO_Port, KP_C2_GPIO_Port};
static const uint16_t      COL_PIN[KEYPAD_COLS]  = {KP_C0_Pin, KP_C1_Pin, KP_C2_Pin};
static GPIO_TypeDef *const ROW_PORT[KEYPAD_ROWS] = {KP_R0_GPIO_Port, KP_R1_GPIO_Port, KP_R2_GPIO_Port, KP_R3_GPIO_Port};
static const uint16_t      ROW_PIN[KEYPAD_ROWS]  = {KP_R0_Pin, KP_R1_Pin, KP_R2_Pin, KP_R3_Pin};

static void hw_drive_col(uint8_t col, bool active)
{
    /* Open-drain output: RESET pulls the column low, SET releases it. */
    HAL_GPIO_WritePin(COL_PORT[col], COL_PIN[col], active ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static uint8_t hw_read_rows(void)
{
    uint8_t rows = 0;

    for (uint8_t r = 0; r < KEYPAD_ROWS; r++) {
        if (HAL_GPIO_ReadPin(ROW_PORT[r], ROW_PIN[r]) == GPIO_PIN_RESET) {
            rows |= (uint8_t)(1u << r);
        }
    }
    return rows;
}

static const keypad_hw_t KEYPAD_HW = {hw_drive_col, hw_read_rows};

/* ---- Access events -> feedback, log, UART ---- */
static void on_access_event(acc_event_t ev, void *user)
{
    (void)user;

    switch (ev) {
    case ACC_EV_KEY:           vib_pulse(VIB_KEY_MS);     break;
    case ACC_EV_PIN_OK:        vib_pulse(VIB_OK_MS);      break;
    case ACC_EV_PIN_BAD:       vib_pulse(VIB_BAD_MS);     break;
    case ACC_EV_LOCKOUT_START: vib_pulse(VIB_LOCKOUT_MS); break;
    default: break;
    }

    if (ev != ACC_EV_KEY) {   /* keystrokes are never logged */
        uint32_t now = HAL_GetTick();

        event_log_add(now, (uint8_t)ev);
#if LOG_EVENTS_TO_UART
        {
            char out[64];

            snprintf(out, sizeof out, "\r\n[%lu ms] %s\r\n> ", (unsigned long)now, access_event_name(ev));
            uart_write(out);
        }
#endif
    }
}

static void update_outputs(uint32_t now)
{
    acc_state_t s = access_state(&g_access);

    if (s != g_last_state) {
        servo_set_us((s == ACC_UNLOCKED) ? SERVO_UNLOCKED_US : SERVO_LOCKED_US);
        g_last_state = s;
    }

    switch (s) {
    case ACC_UNLOCKED: led_set(LED_GREEN); break;
    case ACC_LOCKOUT:  led_set(((now / 250u) & 1u) ? LED_RED : LED_OFF); break;   /* 2 Hz blink */
    default:           led_set(LED_RED); break;
    }
}

/* ---- Public API ---- */
void app_init(void)
{
    char pin[PIN_MAX_LEN + 1u];

    actuators_init();
    keypad_init(&g_keypad, &KEYPAD_HW);
    ringbuf_init(&g_rx, g_rx_storage, (uint16_t)sizeof g_rx_storage);

    access_init(&g_access, pin_store_load(pin, sizeof pin) ? pin : PIN_DEFAULT, on_access_event, NULL);
    shell_init(&g_shell, &g_access, uart_write);

    servo_set_us(SERVO_LOCKED_US);
    uart_write("\r\nkeypad access controller ready - type 'help'\r\n> ");
    (void)HAL_UART_Receive_IT(&APP_UART, &g_rx_byte, 1);
}

void app_poll(void)
{
    uint32_t now = HAL_GetTick();
    char key;
    uint8_t b;

    while (keypad_get_key(&g_keypad, &key)) {
        access_on_key(&g_access, key, now);
    }
    access_tick(&g_access, now);

    while (ringbuf_get(&g_rx, &b)) {
        shell_feed(&g_shell, b, now);
    }
    update_outputs(now);
}

void app_tick_1ms(void)
{
    keypad_tick(&g_keypad);
    actuators_tick_1ms();
}
