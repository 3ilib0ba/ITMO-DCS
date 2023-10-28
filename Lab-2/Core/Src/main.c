/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>


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
uint16_t GREEN = GPIO_PIN_13;
uint16_t YELLOW = GPIO_PIN_14;
uint16_t RED = GPIO_PIN_15;
uint16_t BLINKING_GREEN = 0;
uint8_t INT_ON = 1;
uint8_t INT_OFF = 0;

uint16_t current_light = 1;
uint32_t start_time;
uint32_t duration = 3000;
uint32_t duration_for_red;
uint32_t duration_for_yellow = 3000;
uint32_t blink_duration = 500;
uint32_t blink_count = 6;
uint8_t button_flag = 0;

uint8_t interrupts_mode = 1;
uint8_t is_writing_now = 0;
char read_buffer[100];
char write_buffer[100];
uint8_t status;
char *cur_process_char = read_buffer;
char *cur_read_char = read_buffer;
char *transmit_from_pointer = write_buffer;
char *write_to_pointer = write_buffer;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void wait(uint32_t duration) {
    uint32_t startTime = HAL_GetTick();
    while ((HAL_GetTick() - startTime) < duration) {
    }
}


void turn_all_off() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
}

void turn_specific_light_on(uint16_t light) {
    turn_all_off();
    HAL_GPIO_WritePin(GPIOD, light, GPIO_PIN_SET);
}

void blink_specific_light(uint32_t count, uint16_t light, uint32_t blinkDuration) {
    for (uint32_t i = 0; i < count; i++) {

        turn_all_off();
        wait(blinkDuration);

        turn_specific_light_on(light);
        wait(blinkDuration);
    }
}

int check_starts_with(char *a, char *b) {
    size_t i = 0;
    while (a[i] != '\0') {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return 1;
}

char *concat(char *s1, char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void next(char **pointer, char *buffer) {
    if (*pointer >= buffer + 100) {
        *pointer = buffer;
    } else {
        (*pointer)++;
    }
}

void write_char_to_buff(char c) {
    *write_to_pointer = c;
    next(&write_to_pointer, write_buffer);
}

void write(char *str) {
    char *str_with_newlines = concat("\r\n", str);
    str_with_newlines = concat(str_with_newlines, "\r\n");
    int size = sizeof(char) * strlen(str_with_newlines);
    if (interrupts_mode == 0) {
        HAL_UART_Transmit(&huart6, (uint8_t *) str_with_newlines, size, 10);
    } else {
        for (size_t i = 0; str_with_newlines[i] != '\0'; i++) {
            write_char_to_buff(str_with_newlines[i]);
        }
    }
}

void write_about_info_command() {
    char answer[100];
    char *light;
    switch (current_light) {
        case GPIO_PIN_15:
            light = "red";
            break;
        case GPIO_PIN_14:
            light = "yellow";
            break;
        case GPIO_PIN_13:
            light = "green";
            break;
        case 0:
            light = "blinking green";
            break;
    }
    uint8_t mode;
    char interrupts;
    if (button_flag == 0) {
        mode = 1;
    } else {
        mode = 2;
    }
    if (interrupts_mode == 1) {
        interrupts = 'I';
    } else {
        interrupts = 'P';
    }

    sprintf(answer, "Light: %s, Mode: %d, Timeout: %d, Interrupts: %c", light, mode, duration * 4, interrupts);
    write(answer);
}

void write_command_not_found() {
    char *str = "Invalid command. You can use: '?', set mode 1/2, set timeout X, set interrupts on/off";
    write(str);
}

int is_number(char *str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

void process_symbol() {
    if (*cur_process_char == '\r') {
        *cur_process_char = '\0';


        char *command = strtok(read_buffer, " ");
        if (strcmp(command, "?") == 0) {
            write_about_info_command();
        } else if (strcmp(command, "set") == 0) {
            char *first_arg = strtok(NULL, " ");
            if (strcmp(first_arg, "mode") == 0) {
                char *mode = strtok(NULL, " ");
                if (strcmp(mode, "1") == 0) {
                    button_flag = 0;
//                    duration_for_red = duration * 4;
                    write("Set mode 1. Not ignoring btn");
                } else if (strcmp(mode, "2") == 0) {
                    button_flag = 1;
                    duration_for_red = 4 * duration;
                    write("Set mode 2. Ignored btn at all.");
                } else {
                    write_command_not_found();
                }
            } else if (strcmp(first_arg, "timeout") == 0) {
                char *timeout = strtok(NULL, " ");
                if (is_number(timeout)) {
                    int new_dur = atoi(timeout) * 1000;
                    if (duration_for_red == duration) {
                        duration_for_red = new_dur / 4;
                    } else {
                        duration_for_red = new_dur;
                    }
                    write(concat("New duration is ", timeout));
                    duration = new_dur / 4;
                } else {
                    write_command_not_found();
                }
            } else if (strcmp(first_arg, "interrupts") == 0) {
                char *interrupts = strtok(NULL, " ");
                if (strcmp(interrupts, "on") == 0) {
                    interrupts_mode = 1;
                    transmit_from_pointer = write_to_pointer;
                    cur_read_char = read_buffer;
                    write("Interrupt mode on");
                    HAL_UART_Receive_IT(&huart6, (uint8_t *) cur_read_char, sizeof(char));
                } else if (strcmp(interrupts, "off") == 0) {
                    interrupts_mode = 0;
                    HAL_UART_Abort_IT(&huart6);
                    write("Interrupt mode off");
                } else {
                    write_command_not_found();
                }
            } else {
                write_command_not_found();
            }
        } else {
            write_command_not_found();
        }
        cur_process_char = read_buffer;
    } else {
        next(&cur_process_char, read_buffer);
    }
}

//
//void turn_red_light_on() {
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
//}
//
//void turn_yellow_light_on() {
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
//}
//
//void turn_red_and_yellow_lights_off() {
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
//}
//
//void turn_green_light_on() {
//    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
//}
//
void turn_green_light_off() {
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}

//
//uint32_t get_passed_time(uint32_t startLoopTime) {
//    return HAL_GetTick() - startLoopTime;
//}
//
uint8_t get_BTN() {
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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
    MX_USART6_UART_Init();
    /* USER CODE BEGIN 2 */

    duration_for_red = 4 * duration;
    _Bool nBTN = 0;
    if (interrupts_mode == 1) {
        HAL_UART_Receive_IT(&huart6, (uint8_t *) cur_read_char, sizeof(char));
    }
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        //HAL_UART_Receive_IT(&huart6, (uint8_t *) s, sizeof( char ));
        //HAL_UART_Transmit_IT( &huart6, (uint8_t *) s, sizeof( s ));
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        switch (interrupts_mode) {
            case 1:
                if (cur_process_char != cur_read_char) {
                    process_symbol();
                }
                if (is_writing_now == 0) {
                    if (transmit_from_pointer != write_to_pointer) {
                        is_writing_now = 1;
                        HAL_UART_Transmit_IT(&huart6, (uint8_t *) transmit_from_pointer, sizeof(char));
                    }
                }
                break;
            case 0:
                status = HAL_UART_Receive(&huart6, (uint8_t *) cur_process_char, sizeof(char), 100);
                if (status == HAL_OK) {
                    HAL_UART_Transmit(&huart6, (uint8_t *) cur_process_char, sizeof(char), 10);
                    process_symbol();
                }
                break;
        }

        switch (current_light) {
            case GPIO_PIN_15:
                if (!nBTN && get_BTN() == 0 && button_flag == 0) {
                    nBTN = 1;
                    duration_for_red = duration;
                }
                if ((HAL_GetTick() - start_time) >= duration_for_red) {
                    current_light = GREEN;
                    duration_for_red = 4 * duration;
                    nBTN = 0;
                    turn_specific_light_on(GREEN);
//                    write("start to delay of 10 sec\n");
//                    HAL_Delay(10000);
//                    write("end of waiting HAL_Delay()\n");
                    start_time = HAL_GetTick();
                }
                break;
            case GPIO_PIN_14:
                if (!nBTN && get_BTN() == 0 && button_flag == 0) {
                    nBTN = 1;
                    duration_for_red = duration;
                }
                if ((HAL_GetTick() - start_time) >= duration_for_yellow) {
                    current_light = RED;
                    turn_specific_light_on(RED);
                    start_time = HAL_GetTick();
                }
                break;
            case GPIO_PIN_13:
                if ((HAL_GetTick() - start_time) >= duration) {
                    current_light = BLINKING_GREEN;
//                    blink_count = 3;
                    turn_all_off();
                    start_time = HAL_GetTick();
                }
                break;
            case 0:
                if (!nBTN && get_BTN() == 0 && button_flag == 0) {
                    nBTN = 1;
                    duration_for_red = duration;
                }
                if ((HAL_GetTick() - start_time) >= blink_duration) {
                    if (blink_count > 0) {
                        if (blink_count % 2 == 0) {
                            turn_specific_light_on(GREEN);
                            blink_count--;
                            start_time = HAL_GetTick();
                        } else {
                            turn_green_light_off();
                            blink_count--;
                            start_time = HAL_GetTick();
                        }
                    } else {
                        blink_count = 6;
                        current_light = YELLOW;
                        turn_specific_light_on(YELLOW);
                        start_time = HAL_GetTick();
                    }
                }
                break;
            default:
                current_light = RED;
                turn_specific_light_on(RED);
                start_time = HAL_GetTick();
        }
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
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
    RCC_OscInitStruct.PLL.PLLM = 15;
    RCC_OscInitStruct.PLL.PLLN = 216;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Activate the Over-Drive mode
    */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == huart6.Instance) {
        if (*cur_read_char == '\r') {
            cur_read_char = read_buffer;
        } else {
            write_char_to_buff(*cur_read_char);
            next(&cur_read_char, read_buffer);
        }
        if (interrupts_mode == 1)
            HAL_UART_Receive_IT(&huart6, (uint8_t *) cur_read_char, sizeof(char));
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == huart6.Instance) {
        is_writing_now = 0;
        next(&transmit_from_pointer, write_buffer);
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
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
