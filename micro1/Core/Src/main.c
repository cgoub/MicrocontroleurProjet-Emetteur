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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
DAC_HandleTypeDef hdac;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
int MODE = 0; //to change between the buzzer and the light
char receivedMessage[20]=""; //the message
char morseCode[70]=""; //the message in morse

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_DAC_Init(void);
/* USER CODE BEGIN PFP */
void Switch_Mode(void);
void convertToMorse(char*, char* , int );
void transmitSound(char*);
void transmitLed(char*);
void init(char* ,int );
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


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
  MX_DAC_Init();
  /* USER CODE BEGIN 2 */

    //HAL_UART_Receive_IT(&huart2, (uint8_t *) receivedMessage, 10);
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1){
        /* USER CODE END WHILE */

    	/* receiving the message */
    	init(receivedMessage,10);
    	HAL_UART_Receive(&huart2,(uint8_t *) receivedMessage,20,5000);
    	if(receivedMessage[0]!='\0'){
        	init(morseCode,70);
        	convertToMorse(receivedMessage, morseCode, sizeof(morseCode));
    	}

    	if(MODE==1){
    		/* CODE SOUND */
    		transmitSound(morseCode);
    	}
    	else{
    		/* CODE INFRAROUGE */
    		transmitLed(morseCode);
    	}

    	  /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
  }

  void init(char* message,int maxLength){
	  for(int i=0; i<maxLength;i++){
		  message[i]='\0';
	  }
  }


/**
 * This function converts a string message to Morse code.
 * @param message The string message to be converted.
 * @param morseCode The buffer to store the Morse code.
 * @param maxLength The maximum length of the Morse code buffer.
 */
  void convertToMorse(char* message, char* morseCode, int maxLength)
  {
      // the Morse code alphabet
      const char* morseAlphabet[] = {     "......", ".....-", "....-.", "....--", "...-..", "...-.-", "...--.", "...---", "..-...", "..-..-",
			    "..-.-.", "..-.--", "..--..", "..--.-", "..----", ".-....", ".-...-", ".-..-.", ".-..--", ".-.-..",
			    ".-.-.-", ".-.--.", ".-.---", ".--...", ".--..-", ".--.-.", ".--.--", ".---..", ".---.-", ".----.",
			    ".-----", "-.....", "-....-", "-...-.", "-...--", "-..-.."};

      // Iterate through the message characters
      int morseIndex = 0;

      for (int i = 0; message[i] != '\0' && morseIndex < maxLength - 5; i++) {
          // Get the uppercase representation of the character
          char uppercaseChar = toupper(message[i]);

          // If it's a space, add a space to the Morse code
          if (uppercaseChar == ' ') {
              strcat(morseCode, "/"); // '/' to represent word spacing
              morseIndex += 3; // Move to the next position in morseCode
          }
          else if (isalnum(uppercaseChar)) {
              // Check if it's an alphanumeric character
              if (isalpha(uppercaseChar)) {
                  // Convert letters to Morse code
                  int index = uppercaseChar - 'A'; // Index for Morse code
                  strcat(morseCode, morseAlphabet[index]);
                  morseIndex += strlen(morseAlphabet[index]);
              } else if (isdigit(uppercaseChar)) {
                  // Convert digits to Morse code
                  int index = uppercaseChar - '0' + 26; // Index for Morse code for number since they start after the 26 letter
                  strcat(morseCode, morseAlphabet[index]);
                  morseIndex += strlen(morseAlphabet[index]);
              }
              // Add a space after each Morse code character
              strcat(morseCode, " ");
              morseIndex += 1;
          }
      }
  }

/**
 * This function transmits the Morse code to the buzzer.
 * @param morseCode The Morse code to be transmitted.
 */
  void transmitSound(char* morseCode){

      int i = 0;
      while (morseCode[i] != '\0') {
          if (morseCode[i] == '.') {
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); // Turn on buzzer for dot
              HAL_Delay(100); // Delay for dot
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); // Turn off buzzer
              HAL_Delay(500); // Delay between dot and next signal
          } else if (morseCode[i] == '-') {
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); // Turn on buzzer for dash
              HAL_Delay(300); // Delay for dash
              HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET); // Turn off buzzer
              HAL_Delay(500); // Delay between dash and next signal
          } else if (morseCode[i] == ' ') {
              HAL_Delay(500); // Delay for char gap 1s
          }
          else if (morseCode[i] == '/') {
              HAL_Delay(700); // Delay for word gap 1,2s
          }
          i++;
      }
  }


/**
 * This function transmits the Morse code to the LED.
 * @param morseCode The Morse code to be transmitted.
 */
  void transmitLed(char* morseCode){
      int i = 0;
      while (morseCode[i] != '\0') {
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
		  HAL_Delay(100);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

          if (morseCode[i] == '.') {
              HAL_Delay(300); // Delay for a dot
          } else if (morseCode[i] == '-') {
              HAL_Delay(400); // Delay for a dash
          } else if (morseCode[i] == ' ') {
              // Delay for charecter gap
              HAL_Delay(500); // Delay for character gap
          }
          else if (morseCode[i] == '/') {
              // Delay for word gap
              HAL_Delay(600); // Delay for word gap
          }
          i++;
      }
      HAL_Delay(700); // Delay for word gap
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	  HAL_Delay(250);
	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
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
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */

  /** DAC Initialization
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

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
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
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
