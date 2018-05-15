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

#include "gpio_17xx_40xx.h"

#include <cr_section_macros.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

#define TICKRATE_HZ1 (1000)	// 1000 ticks per second

/* GPIO pin for interrupt */
#define GPIO_INTERRUPT_PIN     17	/* GPIO pin number mapped to interrupt */
#define GPIO_INTERRUPT_PORT    GPIOINT_PORT0	/* GPIO port number mapped to interrupt */

/* On the LPC1769, the GPIO interrupts share the EINT3 vector. */
#define GPIO_IRQ_HANDLER  			EINT3_IRQHandler/* GPIO interrupt IRQ function name */
#define GPIO_INTERRUPT_NVIC_NAME    EINT3_IRQn	/* GPIO interrupt NVIC interrupt name */

// Defines de pulsador
#define	NO		0  							// Defino pulsador no oprimido
#define	SI		1  							// Defino pulsador oprimido
#define	NO_OPRIMIDO		0x00				// Defino estado no oprimido del pulsador
#define	ESPERA_1		0x01				// Defino estado de espera de finalizacion del transitorio del pulsador
#define	ESPERA_2		0x02				// Defino estado de espera de finalizacion del transitorio del pulsador
#define	OPRIMIDO		0x03				// Defino estado de finalizacion del transitorio del pulsador
#define PB_PORT			GPIOINT_PORT0		// Puerto del pulsador
#define PB_BIT			GPIO_INTERRUPT_PIN	// Bit del pulsador
#define DELAY_PULSADOR	30					// Tiempo de espera de finalizacion del transitorio del pulsador [ms]

volatile int j=0;			// contador para el delay del parpadeo
volatile int i=0;       	// contador para cambiar de estado
volatile int ciclo = 0;  	// nro de ciclo
volatile int pulsador = NO;  				// estado de pulsador
volatile int estado_pulsador = NO;  		// estado de pulsador
volatile int pulsador_aux = 0;  			// estado de pulsador leido del puerto
volatile int flag_pulsador = 0;  			// Flag de interrupcion IRQ de pulsador
volatile int pulsador_tick = DELAY_PULSADOR;  		// Contador de espera de finalizacion del transitorio del pulsador


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

	/* Configure GPIO interrupt pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GPIO_INTERRUPT_PORT, GPIO_INTERRUPT_PIN);

	/* Configure the GPIO interrupt */
	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIO_INTERRUPT_PORT, 1 << GPIO_INTERRUPT_PIN);

	/* Configure GPIO interrupt pin as pull up */
	Chip_IOCON_PinMux(LPC_IOCON, 0, 17, IOCON_MODE_PULLUP, IOCON_FUNC0);

	/* Enable interrupt in the NVIC */
	NVIC_ClearPendingIRQ(GPIO_INTERRUPT_NVIC_NAME);
	NVIC_EnableIRQ(GPIO_INTERRUPT_NVIC_NAME);

#endif
#endif

	while(1){

		switch(estado_pulsador){

			case NO_OPRIMIDO:
				if(flag_pulsador){
					flag_pulsador = 0;
					pulsador_tick = DELAY_PULSADOR;
					estado_pulsador = ESPERA_1;
				}break;

			case ESPERA_1:
				if(!pulsador_tick){
					pulsador_aux = Chip_GPIO_ReadValue(LPC_GPIO, PB_PORT) & (1<<PB_BIT);
					if(pulsador_aux){
						estado_pulsador = NO_OPRIMIDO;

						// Configuro interrupcion en pin pulsador por flanco descendente
						Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIO_INTERRUPT_PORT, 1 << GPIO_INTERRUPT_PIN);
						// habilito interrupcion en pin pulsador
						NVIC_EnableIRQ(GPIO_INTERRUPT_NVIC_NAME);
					}
					else{
						estado_pulsador = OPRIMIDO;

						// Configuro interrupcion en pin pulsador por flanco ascendente
						Chip_GPIOINT_SetIntRising(LPC_GPIOINT, GPIO_INTERRUPT_PORT, 1 << GPIO_INTERRUPT_PIN);
						// habilito interrupcion en pin pulsador
						NVIC_EnableIRQ(GPIO_INTERRUPT_NVIC_NAME);
					}
				}break;

			case OPRIMIDO:
				if(flag_pulsador){
					flag_pulsador = 0;
					pulsador_tick = DELAY_PULSADOR;
					estado_pulsador = ESPERA_2;
				}break;

			case ESPERA_2:
				if(!pulsador_tick){
					pulsador_aux = Chip_GPIO_ReadValue(LPC_GPIO,PB_PORT) & (1<<PB_BIT);
					if(pulsador_aux){
						estado_pulsador = OPRIMIDO;
						// Configuro interrupcion en pin pulsador por flanco ascendente
						Chip_GPIOINT_SetIntRising(LPC_GPIOINT, GPIO_INTERRUPT_PORT, 1 << GPIO_INTERRUPT_PIN);
						// habilito interrupcion en pin pulsador
						NVIC_EnableIRQ(GPIO_INTERRUPT_NVIC_NAME);
					}
					else{
						estado_pulsador = NO_OPRIMIDO;
						pulsador = SI;
						// Configuro interrupcion en pin pulsador por flanco descendente
						Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIO_INTERRUPT_PORT, 1 << GPIO_INTERRUPT_PIN);
						// habilito interrupcion en pin pulsador
						NVIC_EnableIRQ(GPIO_INTERRUPT_NVIC_NAME);
					}
				}break;

			default:
				break;
		}

		if (pulsador == SI){
			ciclo++; 						// Cambio de ciclo cada
			j = 0;
			pulsador = NO;					// Normalizo pulsador
		}

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
//	i++;
	//Cambio de ciclo cada 3 segundos
//	if(i==3000){
//		//reseteo los contadores
//		i=0;
//		j=0;
//		// incremento el ciclo
//		ciclo++;
//		if(ciclo==4)ciclo=0;
//		}
	if(pulsador_tick>0){
		pulsador_tick--;
	}

}

/**
 * @brief	Handle interrupt from GPIO pin or GPIO pin mapped to PININT
 * @return	Nothing
 */
void GPIO_IRQ_HANDLER(void)
{
	Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIO_INTERRUPT_PORT, 1 << GPIO_INTERRUPT_PIN);
	flag_pulsador = 1;						// Levanto flag de interrupcion de pulsador
}
