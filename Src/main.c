/**
 ******************************************************************************
 * @file    FLASH/FLASH_EraseProgram/Src/main.c
 * @author  MCD Application Team
 * @brief   This example provides a description of how to erase and program the
 *          STM32F4xx FLASH.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "HexQueue.h"

/** @addtogroup STM32F4xx_HAL_Examples
 * @{
 */

/** @addtogroup FLASH_EraseProgram
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_2   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ADDR_FLASH_SECTOR_7  +  GetSectorSize(ADDR_FLASH_SECTOR_7) -1 /* End @ of user Flash area : sector start address + sector size -1 */

#define DATA_32                 ((uint32_t)0x12345678)





/* Private macro -------------------------------------------------------------*/
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* Private variables ---------------------------------------------------------*/
uint32_t FirstSector = 0, NbOfSectors = 0;
uint32_t Address = 0, SECTORError = 0;
__IO uint32_t data32 = 0 , MemoryProgramStatus = 0;

UART_HandleTypeDef UartHandle; //UART handler declaration

/*Variable used for Erase procedure*/
static FLASH_EraseInitTypeDef EraseInitStruct;

TIM_HandleTypeDef htim7;



/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static uint32_t GetSector(uint32_t Address);
static uint32_t GetSectorSize(uint32_t Sector);
void RTOSInit();
/* Private functions ---------------------------------------------------------*/

static void MX_TIM7_Init(void)
{

	/* USER CODE BEGIN TIM7_Init 0 */

	/* USER CODE END TIM7_Init 0 */

	TIM_MasterConfigTypeDef sMasterConfig = {0};

	/* USER CODE BEGIN TIM7_Init 1 */

	/* USER CODE END TIM7_Init 1 */
	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 179;//was 17999 for 1s
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 49; //was 4999 for 1s
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
	{
		for(;;);
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
	{
		for(;;);
	}
	/* USER CODE BEGIN TIM7_Init 2 */

	/* USER CODE END TIM7_Init 2 */

}

void TinyBLInit(void) {
	HAL_Init();

	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		for(;;);
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		for(;;);
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		for(;;);
	}


	/*GPIO_InitTypeDef  GPIO_InitStruct;
	//##-1- Enable peripherals and GPIO Clocks #################################
	// Enable GPIO TX/RX clock
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	// Enable USARTx clock
	__HAL_RCC_USART3_CLK_ENABLE();

	//##-2- Configure peripheral GPIO ##########################################
	// UART TX GPIO pin configuration
	GPIO_InitStruct.Pin       = GPIO_PIN_8;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	// UART RX GPIO pin configuration
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);*/


	UartHandle.Instance        = USART3;
	UartHandle.Init.BaudRate   = 57600;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		//Initialization Error
		for(;;);
	}

}

uint32_t erasedSectors[8];
uint8_t erasedSectorsLen = 0;
//erase sector if not already erased
void FlashEraseSectorIfNeeded(uint32_t addr) {
	//get current flash sector
	uint32_t sector = GetSector(addr);
	for(uint8_t i = 0; i < erasedSectorsLen; i++) {
		if(sector == erasedSectors[i])
			return;
	}

	//erasing sector
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector        = sector;
	EraseInitStruct.NbSectors     = 1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
	{

		//Error occurred while sector erase.
		//User can add here some code to deal with this error.
		//SECTORError will contain the faulty sector and then to know the code error on this sector,
		//user can call function 'HAL_FLASH_GetError()'
		while (1) {
			BSP_LED_On(LED3);
		}
	}
	erasedSectors[erasedSectorsLen] = sector;
	erasedSectorsLen++;
	return;
}



//process hex and flash
//returns 0 if success, 1 if end of flashing, -1 if error
/*uint8_t ProcessHexFlash(uint8_t *hex) {
	uint8_t dataLen = hex[0];
	uint8_t cmdType = hex[3];
	if(cmdType == 0) { //data
		uint32_t flashAddr = addrOffset + (((uint32_t)hex[1])<<8) + (uint32_t)hex[2];
		//I'm making a brave assumption here. I'm assuming a single data record won't extend between
		//multiple sectors. Is this a safe assumption? I dunno. But it'll save more clock cycles than
		//a half-baked solution
		FlashEraseSectorIfNeeded(flashAddr);

		for(uint8_t i = 0; i < dataLen; i++) {
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, flashAddr+i, hex[i+4]) != HAL_OK) {
				// Error occurred while writing data in Flash memory.
				//User can add here some code to deal with this error
				while (1)
				{
					BSP_LED_On(LED3);
				}
			}

		}
	}
	else if(cmdType == 1) { //EOF
		return 1;
	}
	else if (cmdType == 2) { //extended segment address
		//should not be used so don't bother
	}
	else if (cmdType == 3) { //start segment address
		//we shouldn't need to care about entry address
	}
	else if (cmdType == 4) { //extended linear address
		//printf("Address Command: %s\r\n", hexCmdBuf);
		addrOffset = (((uint32_t)hex[4]) << 24) + (((uint32_t)hex[5]) << 16);
	}
	else if (cmdType == 5) { //start linear address
		//we shouldn't need to care about entry address
	}
	return 0;
}*/



/**
 * @brief  Main program
 * @param  None
 * @retval None
 */

int main(void)
{  
	TinyBLInit();


	GPIO_InitTypeDef  GPIO_InitStruct;

	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

	/* Initialize LED1, LED2 and LED3 */
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);
	BSP_LED_Init(LED3);

	//periodic timer for scheduler
	RTOSInit();
	MX_TIM7_Init();
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_SR_UIF);
	HAL_TIM_Base_Start_IT(&htim7);


	/* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
	 */
	//HAL_Init();

	/* Configure the system clock to 180 MHz */
	//SystemClock_Config();



	for(;;); //infinite loop

	//UART init
	/*UartHandle.Instance        = USARTx;
	UartHandle.Init.BaudRate   = 57600;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = UART_MODE_TX_RX;
	UartHandle.Init.OverSampling = UART_OVERSAMPLING_8;
	if (HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		//Initialization Error
		BSP_LED_On(LED2);
		for(;;);
	}*/


	/* Unlock the Flash to enable the flash control register access *************/
	/*HAL_FLASH_Unlock(); //this is where I comment out old

	// Erase the user Flash area
    //(area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR)

	// Get the 1st sector to erase
	FirstSector = GetSector(FLASH_USER_START_ADDR);
	// Get the number of sector to erase from 1st sector
	NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;
	// Fill EraseInit structure
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector        = FirstSector;
	EraseInitStruct.NbSectors     = NbOfSectors;


	uint8_t printout[50] = "Ready to receive FLASH data\r\n";
	//HAL_UART_Transmit(&UartHandle, printout, 31, HAL_MAX_DELAY);

	// Infinite loop
	HEXQueue q;
	HEXQueueInit(&q);
	while(1) {
		uint16_t count = 0;
		//HAL_UARTEx_ReceiveToIdle(&UartHandle, uartInBuf, 17, &count, 200);
		if((UartHandle.Instance->SR & UART_FLAG_RXNE) == UART_FLAG_RXNE) {
			HEXQueueAdd(&q, (uint8_t)UartHandle.Instance->DR);
			//asm("nop");
			if(HEXQueueExtractHex(&q, hexBuf)) {
				recMsgCount++;
				HEXQueueInit(&q);
				uint8_t result = ProcessHexFlash();
				if(result == 1)
					break;
			}
		}

	}
	HAL_FLASH_Lock();
	while (1)
	{
		HAL_Delay(500);
		BSP_LED_On(LED1);
		//BSP_LED_On(LED2);
		BSP_LED_On(LED3);
		HAL_Delay(500);
		BSP_LED_Off(LED1);
		//BSP_LED_Off(LED2);
		BSP_LED_Off(LED3);
	}*/
}

/**
 * @brief  Gets the sector of a given address
 * @param  None
 * @retval The sector of a given address
 */
static uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
	{
		sector = FLASH_SECTOR_0;
	}
	else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
	{
		sector = FLASH_SECTOR_1;
	}
	else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
	{
		sector = FLASH_SECTOR_2;
	}
	else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
	{
		sector = FLASH_SECTOR_3;
	}
	else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
	{
		sector = FLASH_SECTOR_4;
	}
	else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
	{
		sector = FLASH_SECTOR_5;
	}
	else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
	{
		sector = FLASH_SECTOR_6;
	}
	else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_7) */
	{
		sector = FLASH_SECTOR_7;
	}
	return sector;
}



/**
 * @brief  Gets sector Size
 * @param  None
 * @retval The size of a given sector
 */
static uint32_t GetSectorSize(uint32_t Sector)
{
	uint32_t sectorsize = 0x00;
	if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
	{
		sectorsize = 16 * 1024;
	}
	else if(Sector == FLASH_SECTOR_4)
	{
		sectorsize = 64 * 1024;
	}
	else
	{
		sectorsize = 128 * 1024;
	}
	return sectorsize;
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 180000000
 *            HCLK(Hz)                       = 180000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 8000000
 *            PLL_M                          = 8
 *            PLL_N                          = 360
 *            PLL_P                          = 2
 *            PLL_Q                          = 7
 *            PLL_R                          = 2
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 5
 * @param  None
 * @retval None
 */
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_StatusTypeDef ret = HAL_OK;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	RCC_OscInitStruct.PLL.PLLR = 2;

	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if(ret != HAL_OK)
	{
		while(1) { ; }
	}

	/* Activate the OverDrive to reach the 180 MHz Frequency */
	ret = HAL_PWREx_EnableOverDrive();
	if(ret != HAL_OK)
	{
		while(1) { ; }
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
	if(ret != HAL_OK)
	{
		while(1) { ; }
	}
}

//Task 0. Reads hex records from UART.
//Places received data into inQueue
HEXQueue inQueue;
void UARTInTask() {
	for(;;) {
		if((UartHandle.Instance->SR & UART_FLAG_RXNE) == UART_FLAG_RXNE) {
			HEXQueueAdd(&inQueue, (uint8_t)UartHandle.Instance->DR);
		}
	}
}

//Task 1. Finds Hex records in inQueue.
//Places received data into flashReadyBuf
//flashReadyAddrs stores start address of flashReadyBuf
//flashReadyCount stores number of bytes in flashReadyBuf
#define FLASH_BUF_COUNT 0xFF
#define FLASH_BUF_SIZE 0x10
uint8_t flashReadyBuf[FLASH_BUF_COUNT][FLASH_BUF_SIZE];
uint32_t flashReadyAddrs[FLASH_BUF_COUNT];
uint8_t flashReadyCount[FLASH_BUF_COUNT];
uint32_t recMsgCount = 0; //number of hex messages received. For debug.
uint8_t hexProcBuf[0xff]; //temp buffer for processing hex commands
uint32_t addrOffset = 0; //hex command address offset
void HexProcessTask() {
	for(;;) {
		if(HEXQueueExtractHex(&inQueue, hexProcBuf)) {
			recMsgCount++;

			uint8_t dataLen = hexProcBuf[0];
			uint8_t cmdType = hexProcBuf[3];
			if(cmdType == 0) { //data. Add to buffer
				//find unused buffer idx
				uint16_t cidx;
				for(cidx = 0; cidx < FLASH_BUF_COUNT; cidx++) {
					if(flashReadyAddrs[cidx] == 0 && flashReadyCount[cidx] == 0)
						break;
				}
				if(cidx == FLASH_BUF_COUNT)
					continue; //critical error. Buffer full.

				//fill in data to cache
				for(uint8_t i = 0; i < dataLen; i++)
					flashReadyBuf[cidx][i] = hexProcBuf[i+4];
				flashReadyCount[cidx] = dataLen;
				flashReadyAddrs[cidx] = addrOffset + (((uint32_t)hexProcBuf[1])<<8) + (uint32_t)hexProcBuf[2];
			}
			else if(cmdType == 1) { //EOF
				break;
			}
			else if (cmdType == 2) { //extended segment address
				//should not be used so don't bother
			}
			else if (cmdType == 3) { //start segment address
				//we shouldn't need to care about entry address
			}
			else if (cmdType == 4) { //extended linear address
				//printf("Address Command: %s\r\n", hexCmdBuf);
				addrOffset = (((uint32_t)hexProcBuf[4]) << 24) + (((uint32_t)hexProcBuf[5]) << 16);
			}
			else if (cmdType == 5) { //start linear address
				//we shouldn't need to care about entry address
			}
		}
	}
	while (1)
	{
		HAL_Delay(500);
		BSP_LED_On(LED1);
		//BSP_LED_On(LED2);
		BSP_LED_On(LED3);
		HAL_Delay(500);
		BSP_LED_Off(LED1);
		//BSP_LED_Off(LED2);
		BSP_LED_Off(LED3);
	}
}

//Task 2. Flashing task.
//Flashes data from flashReadyBuf
void FlashTask() {
	HAL_FLASH_Unlock();
	for(;;) {
		for(uint16_t cidx = 0; cidx < FLASH_BUF_COUNT; cidx++) {
			if(flashReadyAddrs[cidx] != 0 && flashReadyCount[cidx] != 0) {
				FlashEraseSectorIfNeeded(flashReadyAddrs[cidx]);

				for(uint8_t i = 0; i < flashReadyCount[cidx]; i++) {
					if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, flashReadyAddrs[cidx]+i, flashReadyBuf[cidx][i]) != HAL_OK) {
						// Error occurred while writing data in Flash memory.
						//User can add here some code to deal with this error
						while (1)
						{
							BSP_LED_On(LED3);
						}
					}
				}
				flashReadyCount[cidx] = 0;
				flashReadyAddrs[cidx] = 0;
			}
		}
	}
}

uint32_t TaskEntryAddrs[3] = {(uint32_t)UARTInTask, (uint32_t)HexProcessTask, (uint32_t)FlashTask};
volatile uint8_t currentTask = 0;
uint8_t T0Stack[0x2000];
uint8_t T1Stack[0x2000];
uint8_t T2Stack[0x2000];
uint32_t TaskSPs[3];
uint8_t firstGo = 1;
void TIM7_IRQHandler(void) {
	__HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
	if(firstGo) { //set stack pointer for task 1
		asm volatile("ldr sp, %0" : : "m" (*(TaskSPs+currentTask)));
		firstGo = 0;
	}
	asm volatile("PUSH	{R4-R11}");
	asm volatile("str sp, %0" : "=m" (*(TaskSPs+currentTask)));
	//scheduling algorithm
	if((UartHandle.Instance->SR & UART_FLAG_RXNE) == UART_FLAG_RXNE)
		currentTask = 0;
	else
		currentTask = (currentTask+1)%3;
	asm volatile("ldr sp, %0" : : "m" (*(TaskSPs+currentTask)));
	asm volatile("POP	{R4-R11}");
}

void RTOSInit() {
	TaskSPs[0] = (uint32_t*)(T0Stack+(0x1000-32));
	*(uint32_t*)(T0Stack+0xFFC) = 0x1000000; //program status
	*(uint32_t*)(T0Stack+0xFF8) = TaskEntryAddrs[0]; //program counter
	*(uint32_t*)(T0Stack+0xFF4) = TaskEntryAddrs[0]; //link register
	*(uint32_t*)(T0Stack+0xFF0) = 0; //R12
	*(uint32_t*)(T0Stack+0xFEC) = 0; //R3
	*(uint32_t*)(T0Stack+0xFE8) = 0; //R2
	*(uint32_t*)(T0Stack+0xFE4) = 0; //R1


	TaskSPs[1] = (uint32_t*)(T1Stack+(0x1000-64));
	*(uint32_t*)(T1Stack+0xFFC) = 0x1000000; //program status
	*(uint32_t*)(T1Stack+0xFF8) = TaskEntryAddrs[1]; //program counter
	*(uint32_t*)(T1Stack+0xFF4) = TaskEntryAddrs[1]; //link register
	*(uint32_t*)(T1Stack+0xFF0) = 0; //R12
	*(uint32_t*)(T1Stack+0xFEC) = 0; //R3
	*(uint32_t*)(T1Stack+0xFE8) = 0; //R2
	*(uint32_t*)(T1Stack+0xFE4) = 0; //R1
	*(uint32_t*)(T1Stack+0xFE0) = 0; //R4
	*(uint32_t*)(T1Stack+0xFDC) = 0; //R5
	*(uint32_t*)(T1Stack+0xFD8) = 0; //R6
	*(uint32_t*)(T1Stack+0xFD4) = 0; //R7
	*(uint32_t*)(T1Stack+0xFD0) = 0; //R8
	*(uint32_t*)(T1Stack+0xFCC) = 0; //R9
	*(uint32_t*)(T1Stack+0xFC8) = 0; //R10
	*(uint32_t*)(T1Stack+0xFC4) = 0; //R11

	TaskSPs[2] = (uint32_t*)(T2Stack+(0x1000-64));
	*(uint32_t*)(T2Stack+0xFFC) = 0x1000000; //program status
	*(uint32_t*)(T2Stack+0xFF8) = TaskEntryAddrs[2]; //program counter
	*(uint32_t*)(T2Stack+0xFF4) = TaskEntryAddrs[2]; //link register
	*(uint32_t*)(T2Stack+0xFF0) = 0; //R12
	*(uint32_t*)(T2Stack+0xFEC) = 0; //R3
	*(uint32_t*)(T2Stack+0xFE8) = 0; //R2
	*(uint32_t*)(T2Stack+0xFE4) = 0; //R1
	*(uint32_t*)(T2Stack+0xFE0) = 0; //R4
	*(uint32_t*)(T2Stack+0xFDC) = 0; //R5
	*(uint32_t*)(T2Stack+0xFD8) = 0; //R6
	*(uint32_t*)(T2Stack+0xFD4) = 0; //R7
	*(uint32_t*)(T2Stack+0xFD0) = 0; //R8
	*(uint32_t*)(T2Stack+0xFCC) = 0; //R9
	*(uint32_t*)(T2Stack+0xFC8) = 0; //R10
	*(uint32_t*)(T2Stack+0xFC4) = 0; //R11

	//zero out flash buffer
	for(uint16_t i = 0; i < FLASH_BUF_COUNT; i++) {
		flashReadyAddrs[i] = 0;
		flashReadyCount[i] = 0;
	}
}



#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif


/**
 * @}
 */

/**
 * @}
 */
