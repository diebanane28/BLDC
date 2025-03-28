#include "stm32f4xx_hal.h"

/////////////////////////////////////////////////////////////////////
//  Aktueller Stand:
//  1Hz PWM signal mit 50% dutycycle auf pins PC6,7,8
//  Getestet drei leds blinken
//  Sys_clk ganz unten noch unklar?
////////////////////////////////////////////////////////////////////


// PWM Timer und Kanal-Konfiguration
TIM_HandleTypeDef htim3;  // Timer 3 wird für PWM verwendet

// Funktion zur Initialisierung der PWM für einen Pin
void PWM_Init(TIM_HandleTypeDef *htim, uint32_t channel);

// Systemtakt und GPIOs initialisieren
void SystemClock_Config(void);
void GPIO_Init(void);
void TIM3_Init(void);

int main(void) {
    HAL_Init();               // HAL Bibliothek initialisieren
    GPIO_Init();              // GPIO Pins initialisieren
    TIM3_Init();              // Timer 3 für PWM konfigurieren

    // PWM für PC6, PC7 und PC8 starten
    PWM_Init(&htim3, TIM_CHANNEL_1);
    PWM_Init(&htim3, TIM_CHANNEL_2);
    PWM_Init(&htim3, TIM_CHANNEL_3);

    while (1) {
        // Endlosschleife
    }
}

void PWM_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    TIM_OC_InitTypeDef sConfigOC = {0};
    
    sConfigOC.OCMode = TIM_OCMODE_PWM1; // PWM Modus 1 => Duty Cycle = 1, pause = 0
    sConfigOC.Pulse = 499;              // 50% Duty Cycle (bei einem Periodenwert von 1000)
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    
    if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel) != HAL_OK) {
        // Fehlerbehandlung
    }
    HAL_TIM_PWM_Start(htim, channel); // PWM Signal starten
}

void GPIO_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE(); // Takt für GPIOC aktivieren
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;   // Alternativfunktion für PWM
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3; // Timer3 PWM Funktion
    
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void TIM3_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE(); // Takt für Timer 3 aktivieren
    
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 15999;  // Prescaler, um 1 kHz Zählerfrequenz zu erreichen################IST WIRKLICH 84MHZ?!?!?
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;  // PWM Frequenz = 1 Hz
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
        // Fehlerbehandlung
    }
}

extern "C" 
    void SysTick_Handler(void) { HAL_IncTick(); }
    