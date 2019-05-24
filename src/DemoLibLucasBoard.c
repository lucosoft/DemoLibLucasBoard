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

#include "pulsador_irq.h"
#include "parpadeo.h"
#include "i2c_eeprom.h"
#include "display_1602.h"
#include "teclado_matricial.h"

#include <cr_section_macros.h>

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
#ifdef BOARD_NXP_LPCXPRESSO_1769
#define _ADC_CHANNLE ADC_CH0
#else
#define _ADC_CHANNLE ADC_CH2
#endif
#define _LPC_ADC_ID LPC_ADC
#define _LPC_ADC_IRQ ADC_IRQn
#define _GPDMA_CONN_ADC GPDMA_CONN_ADC
static char WelcomeMenu[] = "\r\nHello NXP Semiconductors \r\n"
							"ADC DEMO \r\n"
#if defined(CHIP_LPC175X_6X)
							"Sample rate : 200kHz \r\n"
#else
							"Sample rate : 400kHz \r\n"
#endif
							"Press \'c\' to continue or \'x\' to quit\r\n"
							"Press \'o\' or \'p\' to set Sample rate\r\n"
							"Press \'b\' to ENABLE or DISABLE Burst Mode\r\n";
static char SelectMenu[] = "\r\nPress number 1-3 to choose ADC running mode:\r\n"
						   "\t1: Polling Mode \r\n"
						   "\t2: Interrupt Mode \r\n"
						   "\t3: DMA Mode \r\n";

static ADC_CLOCK_SETUP_T ADCSetup;
static volatile uint8_t Burst_Mode_Flag = 0, Interrupt_Continue_Flag;
static volatile uint8_t ADC_Interrupt_Done_Flag, channelTC, dmaChannelNum;
uint32_t DMAbuffer;
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Print ADC value and delay */
static void App_print_ADC_value(uint16_t data)
{
	volatile uint32_t j;
	j = 5000000;
	DEBUGOUT("ADC value is : 0x%04x\r\n", data);
	/* Delay */
	while (j--) {}
}

/* DMA routine for ADC example */
static void App_DMA_Test(void)
{
	uint16_t dataADC;

	/* Initialize GPDMA controller */
	Chip_GPDMA_Init(LPC_GPDMA);
	/* Setting GPDMA interrupt */
	NVIC_DisableIRQ(DMA_IRQn);
	NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));
	NVIC_EnableIRQ(DMA_IRQn);
	/* Setting ADC interrupt, ADC Interrupt must be disable in DMA mode */
	NVIC_DisableIRQ(_LPC_ADC_IRQ);
	Chip_ADC_Int_SetChannelCmd(_LPC_ADC_ID, _ADC_CHANNLE, ENABLE);
	/* Get the free channel for DMA transfer */
	dmaChannelNum = Chip_GPDMA_GetFreeChannel(LPC_GPDMA, _GPDMA_CONN_ADC);
	/* Enable burst mode if any, the AD converter does repeated conversions
	   at the rate selected by the CLKS field in burst mode automatically */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(_LPC_ADC_ID, ENABLE);
	}
	/* Get  adc value until get 'x' character */
	while (DEBUGIN() != 'x') {
		/* Start A/D conversion if not using burst mode */
		if (!Burst_Mode_Flag) {
			Chip_ADC_SetStartMode(_LPC_ADC_ID, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
		channelTC = 0;
		Chip_GPDMA_Transfer(LPC_GPDMA, dmaChannelNum,
						  _GPDMA_CONN_ADC,
						  (uint32_t) &DMAbuffer,
						  GPDMA_TRANSFERTYPE_P2M_CONTROLLER_DMA,
						  1);

		/* Waiting for reading ADC value completed */
		while (channelTC == 0) {}

		/* Get the ADC value fron Data register*/
		dataADC = ADC_DR_RESULT(DMAbuffer);
		App_print_ADC_value(dataADC);
	}
	/* Disable interrupts, release DMA channel */
	Chip_GPDMA_Stop(LPC_GPDMA, dmaChannelNum);
	NVIC_DisableIRQ(DMA_IRQn);
	/* Disable burst mode if any */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(_LPC_ADC_ID, DISABLE);
	}
}

/* Interrupt routine for ADC example */
static void App_Interrupt_Test(void)
{
	/* Enable ADC Interrupt */
	NVIC_EnableIRQ(_LPC_ADC_IRQ);
	Chip_ADC_Int_SetChannelCmd(_LPC_ADC_ID, _ADC_CHANNLE, ENABLE);
	/* Enable burst mode if any, the AD converter does repeated conversions
	   at the rate selected by the CLKS field in burst mode automatically */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(_LPC_ADC_ID, ENABLE);
	}
	Interrupt_Continue_Flag = 1;
	ADC_Interrupt_Done_Flag = 1;
	while (Interrupt_Continue_Flag) {
		if (!Burst_Mode_Flag && ADC_Interrupt_Done_Flag) {
			ADC_Interrupt_Done_Flag = 0;
			Chip_ADC_SetStartMode(_LPC_ADC_ID, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
	}
	/* Disable burst mode if any */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(_LPC_ADC_ID, DISABLE);
	}
	/* Disable ADC interrupt */
	NVIC_DisableIRQ(_LPC_ADC_IRQ);
}

/* Polling routine for ADC example */
static void App_Polling_Test(void)
{
	uint16_t dataADC;

	/* Select using burst mode or not */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(_LPC_ADC_ID, ENABLE);
	}
	else {
		Chip_ADC_SetBurstCmd(_LPC_ADC_ID, DISABLE);
	}

	/* Get  adc value until get 'x' character */
	while (DEBUGIN() != 'x') {
		/* Start A/D conversion if not using burst mode */
		if (!Burst_Mode_Flag) {
			Chip_ADC_SetStartMode(_LPC_ADC_ID, ADC_START_NOW, ADC_TRIGGERMODE_RISING);
		}
		/* Waiting for A/D conversion complete */
		while (Chip_ADC_ReadStatus(_LPC_ADC_ID, _ADC_CHANNLE, ADC_DR_DONE_STAT) != SET) {}
		/* Read ADC value */
		Chip_ADC_ReadValue(_LPC_ADC_ID, _ADC_CHANNLE, &dataADC);
		/* Print ADC value */
		App_print_ADC_value(dataADC);
	}

	/* Disable burst mode, if any */
	if (Burst_Mode_Flag) {
		Chip_ADC_SetBurstCmd(_LPC_ADC_ID, DISABLE);
	}
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	ADC0 interrupt handler sub-routine
 * @return	Nothing
 */
void ADC_IRQHandler(void)
{
	uint16_t dataADC;
	/* Interrupt mode: Call the stream interrupt handler */
	NVIC_DisableIRQ(_LPC_ADC_IRQ);
	Chip_ADC_Int_SetChannelCmd(_LPC_ADC_ID, _ADC_CHANNLE, DISABLE);
	Chip_ADC_ReadValue(_LPC_ADC_ID, _ADC_CHANNLE, &dataADC);
	ADC_Interrupt_Done_Flag = 1;
	App_print_ADC_value(dataADC);
	if (DEBUGIN() != 'x') {
		NVIC_EnableIRQ(_LPC_ADC_IRQ);
		Chip_ADC_Int_SetChannelCmd(_LPC_ADC_ID, _ADC_CHANNLE, ENABLE);
	}
	else {Interrupt_Continue_Flag = 0; }
}

/**
 * @brief	DMA interrupt handler sub-routine
 * @return	Nothing
 */
void DMA_IRQHandler(void)
{
	if (Chip_GPDMA_Interrupt(LPC_GPDMA, dmaChannelNum) == SUCCESS) {
		channelTC++;
	}
	else {
		/* Process error here */
	}
}

/**
 * @brief	main routine for examples
 * @return	Function should not exit.
 */
int main(void) {

	int teclado = 0;

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();

	#endif
#endif

	Pulsador_Irq_Init();
	I2C_Eeprom_Init();
	I2C_Eeprom_Escribir();
	I2C_Eeprom_Leer();
	LCD_Init();
	LCD_Print("INICIALIZANDO...");
	Teclado_Init ();
	bool end_Flag = false;
	uint32_t _bitRate = ADC_MAX_SAMPLE_RATE;
	uint8_t bufferUART;

	Pulsador_Irq_Init();
	I2C_Eeprom_Init();
	I2C_Eeprom_Escribir();
	I2C_Eeprom_Leer();

	/*	Chip_IOCON_PinMux(0, 25, IOCON_ADMODE_EN, IOCON_FUNC1); */
	/*ADC Init */
	Chip_ADC_Init(_LPC_ADC_ID, &ADCSetup);
	Chip_ADC_EnableChannel(_LPC_ADC_ID, _ADC_CHANNLE, ENABLE);

	while (!end_Flag) {
		DEBUGOUT(WelcomeMenu);
		while (!end_Flag) {
			bufferUART = 0xFF;
			bufferUART = DEBUGIN();
			if (bufferUART == 'c') {
				DEBUGOUT(SelectMenu);
				bufferUART = 0xFF;
				while (bufferUART == 0xFF) {
					bufferUART = DEBUGIN();
					if ((bufferUART != '1') && (bufferUART != '2') && (bufferUART != '3')) {
						bufferUART = 0xFF;
					}
				}
				switch (bufferUART) {
				case '1':		/* Polling Mode */
					App_Polling_Test();
					break;

				case '2':		/* Interrupt Mode */
					App_Interrupt_Test();
					break;

				case '3':		/* DMA mode */
					App_DMA_Test();
					break;
				}
				break;
			}
			else if (bufferUART == 'x') {
				end_Flag = true;
				DEBUGOUT("\r\nADC demo terminated!");
			}
			else if (bufferUART == 'o') {
				_bitRate -= _bitRate > 0 ? 1000 : 0;
				Chip_ADC_SetSampleRate(_LPC_ADC_ID, &ADCSetup, _bitRate);
				DEBUGOUT("Rate : %d Sample/s\r\n", _bitRate);
			}
			else if (bufferUART == 'p') {
				_bitRate += _bitRate < 400000 ? 1000 : 0;
				Chip_ADC_SetSampleRate(_LPC_ADC_ID, &ADCSetup, _bitRate);
				DEBUGOUT("Rate : %d Sample/s\r\n", _bitRate);
			}
			else if (bufferUART == 'b') {
				Burst_Mode_Flag = !Burst_Mode_Flag;
				ADCSetup.burstMode = Burst_Mode_Flag;
				Chip_ADC_SetSampleRate(_LPC_ADC_ID, &ADCSetup, _bitRate);
				if (Burst_Mode_Flag) {
					DEBUGOUT("Burst Mode ENABLED\r\n");
				}
				else {
					DEBUGOUT("Burst Mode DISABLED\r\n");
				}
			}
		}
	}

	while (1) {

		Pulsador_Irq_Sec();
		Parpadeo_Sec();
		teclado = Teclado_Presionado();
	}
	return 0;
}


