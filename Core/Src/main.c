/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "i2c.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "RDA5807.h"
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

/* USER CODE BEGIN PV */
uint8_t vol_value = 7; // 音量值
uint8_t chan_cnt = 0;  // 频道数量
float now_chan = 0;    // 当前频道
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void ShowAllChannel(uint8_t chan_cnt);
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
    MX_I2C1_Init();
    MX_I2C2_Init();
    /* USER CODE BEGIN 2 */
    OLED_Init(); // OLED初始化

    // 复位
    RDA5807_Reset();
    // 上电
    RDA5807_PowerOn();
    // 搜台
    OLED_ShowString(1, 1, "Searching...", OLED_8x6);
    chan_cnt = RDA5807_AutoSearch();
    // 显示频道数量
    OLED_Clear();
    OLED_ShowString(1, 1, "Station Count:", OLED_8x6);
    OLED_ShowNum(1, 15, chan_cnt, 2, OLED_8x6);
    // 显示所有频道
    ShowAllChannel(chan_cnt);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        // 音量调节
        if (HAL_GPIO_ReadPin(VOL_UP_GPIO_Port, VOL_UP_Pin) == GPIO_PIN_RESET)
        {
            HAL_Delay(20);
            while (HAL_GPIO_ReadPin(VOL_UP_GPIO_Port, VOL_UP_Pin) == GPIO_PIN_RESET)
                ;
            HAL_Delay(20);
            if (vol_value < 15)
                vol_value++;
            else
                vol_value = 15;
            RDA5807_SetVolume(vol_value);
            OLED_ShowString(7, 1, "Volume:          ", OLED_8x6);
            OLED_ShowNum(7, 9, vol_value, 2, OLED_8x6);
        }
        if (HAL_GPIO_ReadPin(VOL_DOWM_GPIO_Port, VOL_DOWM_Pin) == GPIO_PIN_RESET)
        {
            HAL_Delay(20);
            while (HAL_GPIO_ReadPin(VOL_DOWM_GPIO_Port, VOL_DOWM_Pin) == GPIO_PIN_RESET)
                ;
            HAL_Delay(20);
            if (vol_value > 0)
                vol_value--;
            else
                vol_value = 0;
            RDA5807_SetVolume(vol_value);
            OLED_ShowString(7, 1, "Volume:          ", OLED_8x6);
            OLED_ShowNum(7, 9, vol_value, 2, OLED_8x6);
        }

        // 换台
        if (HAL_GPIO_ReadPin(CH_UP_GPIO_Port, CH_UP_Pin) == GPIO_PIN_RESET)
        {
            HAL_Delay(20);
            while (HAL_GPIO_ReadPin(CH_UP_GPIO_Port, CH_UP_Pin) == GPIO_PIN_RESET)
                ;
            HAL_Delay(20);
            now_chan = RDA5807_ChangeStation(1);
            OLED_ShowString(6, 1, "Now Channel:      ", OLED_8x6);
            OLED_ShowFloat(6, 14, now_chan, OLED_8x6);
        }
        if (HAL_GPIO_ReadPin(CH_DOWN_GPIO_Port, CH_DOWN_Pin) == GPIO_PIN_RESET)
        {
            HAL_Delay(20);
            while (HAL_GPIO_ReadPin(CH_DOWN_GPIO_Port, CH_DOWN_Pin) == GPIO_PIN_RESET)
                ;
            HAL_Delay(20);
            now_chan = RDA5807_ChangeStation(0);
            OLED_ShowString(6, 1, "Now Channel:      ", OLED_8x6);
            OLED_ShowFloat(6, 14, now_chan, OLED_8x6);
        }

        // 重新搜台
        if (HAL_GPIO_ReadPin(AUTOSEARCH_GPIO_Port, AUTOSEARCH_Pin) == GPIO_PIN_RESET)
        {
            HAL_Delay(20);
            while (HAL_GPIO_ReadPin(AUTOSEARCH_GPIO_Port, AUTOSEARCH_Pin) == GPIO_PIN_RESET)
                ;
            HAL_Delay(20);
            // 复位
            chan_cnt = 0;
            for (uint8_t i = 0; i < 25; i++)
            {
                channelBuf[i] = 0.0f;
            }
            
            OLED_Clear();
            OLED_ShowString(1, 1, "Searching...", OLED_8x6);
            chan_cnt = RDA5807_AutoSearch();
            // 显示频道数量
            OLED_Clear();
            OLED_ShowString(1, 1, "Station Count:", OLED_8x6);
            OLED_ShowNum(1, 15, chan_cnt, 2, OLED_8x6);
            // 显示所有频道
            ShowAllChannel(chan_cnt);
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
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
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */

/**
 * @brief  显示所有频道
 * @param  chan_cnt: 频道数量
 */
void ShowAllChannel(uint8_t chan_cnt)
{
    // 显示频道
    for (uint8_t i = 0; i < chan_cnt; i++)
    {
        if (i <= 3)
        {
            OLED_ShowFloat(2, 1 + i * 5, channelBuf[i], OLED_8x6);
        }
        else if (i > 3 && i <= 7)
        {
            OLED_ShowFloat(3, 1 + (i - 4) * 5, channelBuf[i], OLED_8x6);
        }
        else if (i > 7 && i <= 11)
        {
            OLED_ShowFloat(4, 1 + (i - 8) * 5, channelBuf[i], OLED_8x6);
        }
        else if (i > 11 && i <= 15)
        {
            OLED_ShowFloat(5, 1 + (i - 12) * 5, channelBuf[i], OLED_8x6);
        }
        else if (i > 15 && i <= 19)
        {
            OLED_ShowFloat(6, 1 + (i - 16) * 5, channelBuf[i], OLED_8x6);
        }
        else if (i > 19 && i <= 23)
        {
            OLED_ShowFloat(7, 1 + (i - 20) * 5, channelBuf[i], OLED_8x6);
        }
    }
}
/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM4 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM4)
    {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

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

#ifdef USE_FULL_ASSERT
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
