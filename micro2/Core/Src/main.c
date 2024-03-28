/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DOT_THRESHOLD   110     // Adjust according to your requirements
#define DASH_THRESHOLD  310     // Adjust according to your requirements
#define PAUSE_THRESHOLD 1000
#define PAUSE_THRESHOLD_HIGH 1050
#define PAUSE_THRESHOLD_LOW 950
#define GAP_THRESHOLD 1310
#define THRESHOLD_low (uint16_t)1300
#define THRESHOLD_high (uint16_t)1700
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char rx_buffer[100] = "";

uint16_t adc_value = 0;
uint8_t received_byte = 0;
uint32_t startTime=0;
uint32_t notStartTime=0;
uint32_t endTime= 0;
uint32_t duration=0;

int MODE=0;
int rx_index =0;

int haveConverted = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC3_Init(void);
/* USER CODE BEGIN PFP */
void decodeMorse(char* , int );
int isSoundSignalDetected();
int isLedSignalDetected();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small
printf
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */
  printf("The program has correctly been loaded.\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint16_t led=1;
  while (1)
      {
	  if(MODE==1){ //Sound mode
		  notStartTime = HAL_GetTick();
		  int silence_duration = notStartTime - endTime;
		  if (!haveConverted && (silence_duration >= 2000 || rx_index >= sizeof(rx_buffer)))
		  {

			  // Decode the Morse code character
			  decodeMorse(rx_buffer, rx_index);
			  // Reset the message buffer index

			  for(int i=0;i<rx_index;i++){
				  rx_buffer[i] = '\0';
			  }
			  rx_index = 0;
			  haveConverted = 1;
		  }
		  /* Check for sound signal */
		  if (isSoundSignalDetected()) {
			  haveConverted = 0;
			  /* Record start time of signal */

			  if(silence_duration>500){
				  startTime = notStartTime;
			  } else {
				  rx_index--;
			  }
			  while (adc_value < THRESHOLD_low || adc_value > THRESHOLD_high) {
				  // Check if sound signal ended
				HAL_ADC_Start(&hadc3);
				HAL_ADC_PollForConversion(&hadc3, HAL_MAX_DELAY);
				adc_value = HAL_ADC_GetValue(&hadc3);//* (5.0 / 1023.0);
			  }


			  /* Record end time of signal */
			  endTime = HAL_GetTick();

			  /* Calculate duration of signal */
			  duration = (endTime - startTime);

			  /* test if the silence represent a pause between 2 characters*/
			  if (silence_duration > PAUSE_THRESHOLD_LOW && silence_duration < PAUSE_THRESHOLD_HIGH)
			  {
				  rx_buffer[rx_index] = ' ';
				  rx_index++;
			  }
			  /* test if the silence represent a pause between 2 words */
			  else if (silence_duration > PAUSE_THRESHOLD_HIGH && rx_index!=0)
			  {
				  rx_buffer[rx_index] = '/';
				  rx_index++;
			  }

			  /* test if the duration of the signal represent a dot*/
			  if (duration < DOT_THRESHOLD)
			  {
				  rx_buffer[rx_index] = '.';
				  rx_index++;
			  }
			  /* test if the duration of the signal represent a dash*/
			  else if (duration < DASH_THRESHOLD)
			  {
				  rx_buffer[rx_index] = '-';
				  rx_index++;
			  }
		  }
		  HAL_Delay(5);
		  HAL_ADC_Stop(&hadc3);
	  }
	  if(MODE==0){ //Led mode
		  if (!led) {
			  led=1;
			  while (led){ //waiting while the Led is off
				  led=HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
			  }
			  HAL_Delay(100); // When the led is on we wait a little more to calibrate
			  endTime = HAL_GetTick(); // recuperation of the time the led turn on again

			  /* Calculate duration of signal */
			  duration = endTime - startTime - 100; //getting the time the led was off
			  HAL_Delay(5);
			  if (duration> 290 && duration < 310) // 300ms is the time of a "."
			  {
				  rx_buffer[rx_index] = '.';
				  rx_index++;
			  }
			  else if (duration>390 &&duration < 410) // 400ms is the time of a "-"
			  {
				  rx_buffer[rx_index] = '-';
				  rx_index++;
			  }
			  else if (duration>490 &&duration < 510) //500ms is the time of a space between 2 character
			  {
				  rx_buffer[rx_index] = ' ';
				  rx_index++;
			  }
			  else if (duration>590 &&duration < 610) //600ms is the time of a space between 2 words
			  {
				  rx_buffer[rx_index] = '/';
				  rx_index++;
			  }
			  else
			  {
				  // Decode the Morse code character
				  decodeMorse(rx_buffer, rx_index);
				  // Reset the message buffer index

				  for(int i=0;i<rx_index;i++){
					  rx_buffer[i] = '\0';
				  }
				  rx_index = 0;
			  }
			  startTime = endTime;
		  }
		  else{
			  led = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);
			  HAL_Delay(100); // Wait for the moment the Led turn off
			  startTime = HAL_GetTick();
		  }
	  }
   }
}

  /* Function to detect the presence of a sound signal */
	int isSoundSignalDetected() {
		HAL_ADC_Start(&hadc3);
	  	HAL_ADC_PollForConversion(&hadc3, HAL_MAX_DELAY);

	    // Read the analog input from the sound sensor using the ADC
		//HAL_ADC_PollForConversion(&hadc3, 100);
	    adc_value = HAL_ADC_GetValue(&hadc3);//* (5.0 / 1023.0); // Example ADC reading

        /* Wait for sound signal to end */
        HAL_ADC_Stop(&hadc3);

	    // Analyze the ADC value to determine if a sound signal is detected
        if (adc_value < THRESHOLD_low || adc_value > THRESHOLD_high) {
	        return 1; // Sound signal detected
	    } else {
	    	return 0; // No sound signal detected
		}
  }

	int isLedSignalDetected() {
		return !HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
  }


	void decodeMorse(char *message, int length)
	  {
	      //printf("%s\r\n", message);
		  char* morseAlphabet[] = {     "......", ".....-", "....-.", "....--", "...-..", "...-.-", "...--.", "...---", "..-...", "..-..-",
				    "..-.-.", "..-.--", "..--..", "..--.-", "..----", ".-....", ".-...-", ".-..-.", ".-..--", ".-.-..",
				    ".-.-.-", ".-.--.", ".-.---", ".--...", ".--..-", ".--.-.", ".--.--", ".---..", ".---.-", ".----.",
				    ".-----", "-.....", "-....-", "-...-.", "-...--", "-..-.."};

	      char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	      char decoded_message[length + 1]; // Maximum possible length of decoded message

	      int decoded_index = 0; // Index to keep track of the position in the decoded message
	      int k;
	      int i=0;

	      while(i<length){
			  k=0;
			  decoded_index = 0;
	    	  if (message[i] == ' '){
	    		  i++;
	    	  }
	    	  else if (message[i] == '/'){
		    		printf(" ");
	    		    i++;
	    	  }
	    	  else{
	    		  for(decoded_index=0;decoded_index<6;decoded_index++){
	    			  decoded_message[decoded_index] = message[i];
	    			  i++;
	    		  }
				  while(k < sizeof(morseAlphabet) / sizeof(morseAlphabet[0]) && strcmp(decoded_message, morseAlphabet[k]) != 0){
					k++;
				  }
				  printf("%c",alphabet[k]);
	    	  }
	      }
	      printf("\r\n");
	  }


  PUTCHAR_PROTOTYPE
  {
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART2 and Loop until the end of
  transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
  }
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
  /* USER CODE END Error_Handler_Debug */
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
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
