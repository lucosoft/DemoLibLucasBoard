/*
===============================================================================
 Name        : DemoLibLucasBoard.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define TICKRATE_HZ1 (1000)	// 1000 ticks per second

volatile int j=0;			// contador para el delay del parpadeo
volatile int i=0;       	// contador para cambiar de estado
volatile int ciclo = 0;  	// nro de ciclo

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	main routine for examples
 * @return	Function should not exit.
 */
int main(void) {

#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();

    // Enable and setup SysTick Timer at a periodic rate
	SysTick_Config(SystemCoreClock / TICKRATE_HZ1);

    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif

	while(1){
		switch(ciclo){
			case 0:
				// Cada 1 segundo...
				if(j==1000){
					// prender si led estaba apagado o apagar si led estaba prendido
					Board_LED_Toggle(0);
					// reset de la variable contador para el delay
					j = 0;
				}break;
			case 1:
				// Cada 500ms...
				if(j==500){
					// prender si led estaba apagado o apagar si led estaba prendido
					Board_LED_Toggle(0);
					// reset de la variable contador para el delay
					j = 0;
				}break;
			case 2:
				// Cada 500ms...
				if(j==250){
					// prender si led estaba apagado o apagar si led estaba prendido
					Board_LED_Toggle(0);
					// reset de la variable contador para el delay
					j = 0;
				}break;
			case 3:
				// Cada 500ms...
				if(j==125){
					// prender si led estaba apagado o apagar si led estaba prendido
					Board_LED_Toggle(0);
					// reset de la variable contador para el delay
					j = 0;
				}break;
			default:
				// reseteo ciclo y los contadores
				ciclo=0;
				j=0;
				i=0;
				break;
		}
	}
	return 0 ;
}

/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void) {
	// incremento la variable contador para el delay
	j++;
	// incremento la variable contador para cambiar de estado
	i++;
	//Cambio de ciclo cada 3 segundos
	if(i==3000){
		//reseteo los contadores
		i=0;
		j=0;
		// incremento el ciclo
		ciclo++;
		if(ciclo==4)ciclo=0;
		}
}
