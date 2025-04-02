#include "stm32f4xx_hal.h"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstring>
#define SYSTEMTAKT 10e6 //  1 MHz Taktfrequenz am PeripheralBUS wo TIM3 für PWM 
#define TOTAL_ACCELERATION_LOOPS 1000 //totale anzahl an beschleun.-loops => kleiner = schnellerer anlauf


/////////////////////////////////////////////////////////////////////
//  Aktueller Stand:
//  1Hz PWM signal mit 50% dutycycle auf pins PC6,7,8
//  Getestet drei leds blinken
//  Sys_clk ganz unten noch unklar?
////////////////////////////////////////////////////////////////////

//globale Variable
int counter =0;
int total_pole_loops =0;
const int tim3_prescaler = (16*10e6 / SYSTEMTAKT)-1;

// PWM Timer und Kanal-Konfiguration
TIM_HandleTypeDef htim3;  // Timer 3 wird für PWM verwendet

// Funktion zur Initialisierung der PWM für einen Pin
void PWM_Init(TIM_HandleTypeDef *htim, uint32_t channel);

// Systemtakt und GPIOs initialisieren
void SystemClock_Config(void);
void GPIO_Init(void);
void TIM3_Init(void);

//weitere funktionen
void delay(int delaytime);
int startup_ramp_times(int startvalue, int endvalue, int loops, int acc_slower);
//diese gleich hier implementiert weil kompliziert zu deklarieren...
//////////////////////////////////////////////////////////////////////////
const int table_length = 512;
const std::array<int, table_length> fastCos = [] {
    std::array<int, table_length> arr{};
    for (std::size_t i = 0; i < table_length; i++) {
        double x = (static_cast<double>(i) / table_length) * 2 * M_PI;  // x von 0 bis 2π
        arr[i] = static_cast<int>(std::round(128 * cos(x) + 128));  // Wert berechnen & runden
    }
    return arr;
}();
///////////////////////////////////////////////////////////////////////////


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
        if (total_pole_loops < TOTAL_ACCELERATION_LOOPS) {  //  hier zb 1000 Poleloops bis volle Geschwindigkeit erreicht
                                        //  ab dann weiterzählen sinnlos...nur overflow gefahr
            total_pole_loops++; //counts the pole loops
        }
        
        while(counter<256) {
        TIM3->CCR1 = fastCos[counter];   //Capture-Compare-Register toggelt den Pin CCR1 = channel1
        TIM3->CCR2 = fastCos[counter+85]; //-120° Phasenverschub
        TIM3->CCR3 = fastCos[counter+171]; //-240° Phasenverschub
        delay(startup_ramp_times(20000, 400, total_pole_loops, 1));
        counter++;
        }
        counter =0;
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
    htim3.Init.Prescaler = tim3_prescaler;  // Takt siehe #define SYSTEMTAKT (aktuell 1 MHz)
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 256;  // PWM Frequenz = SYSTEMTAKT / 256 = ca 3,90 kHz
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
        // Fehlerbehandlung
    }
}

extern "C" 
    void SysTick_Handler(void) { HAL_IncTick(); }


void delay(int delaytime) {
    int i = 0;
    while (i<delaytime) {
        i++;
    }
}

int startup_ramp_times(int startvalue, int endvalue, int loops, int acc_slower) {
    // total loops bis endvalue: 1000
    // start drehzahl = 0,2 U/s => start_delay = 20 000 Ticks bei 1 MHz
    // beschleunigung = 1 U/s^2
    // enddrehzahl = 10 U/s => end_delay = 400 Ticks bei 1 MHz

    return std::round(acc_slower*(startvalue-endvalue)/(loops) + endvalue);
     
}