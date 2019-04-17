/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "hrtim.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "string.h"
#include <stdlib.h>
#include <stdio.h>
#include "arm_math.h"
#include "NRF24L01.h"
#include "dma_mine.h"
//#include <cstdio>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

void Init_Timers(void);
void Init_PCA9539(void);		//PCA9539��ʼ��
void Buzzer_Ring(void);					//����������
void Buzzer_Off(void);					//�������ر�
void Is_Infrared(void);
void Shoot_Chip(void);
void Is_BatteryLow_BootCharged(void);
void dribber(void);
void pack(uint8_t *);
void PWM_SET_VALUE(TIM_HandleTypeDef *htim,uint32_t Channel,uint16_t value);
void unpack(uint8_t *Packet);
void motion_planner();

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//Robot����
uint8_t tx_freq, tx_mode, rx_freq;								//Ƶ��
uint8_t robot_num;
uint8_t robot_set = 0x08;
uint8_t Robot_Is_Infrared; 				//���ⴥ��
uint8_t Robot_Is_Boot_charged; 		//���ݳ�絽60V
uint8_t Robot_drib;
uint8_t Robot_Chip_Or_Shoot;						//chip:1  shoot:0
uint8_t Robot_Is_Shoot;
uint8_t Robot_Is_Chip;
uint8_t Robot_Boot_Power;
////uint8_t Robot_Is_Report;
uint8_t Robot_Chipped = 0, Robot_Shooted = 0;
uint8_t Robot_Status = 0, Last_Robot_Status = 0;
int8_t Left_Report_Package = 0;
uint16_t drib_power, drib_power_set[4] = {0, 400, 600, 2500};
uint8_t selftest_vel_mode = 0x02, selftest_drib_mode = 0x03, selftest_chip_mode = 0x04, selftest_shoot_mode = 0x05, selftest_discharge_mode = 0x06; 
//ALIGN_32BYTES(__attribute__((section (".RAM_D2"))) )
uint16_t ADC_value[32];   			//���ADC�ɼ�����

//��ص�ѹ����ݵ�ѹ�ۼ�ֵ
uint32_t AD_Battery = 0,AD_Battery_Last = 217586, AD_Boot_Cap = 0, AD_Boot_Cap_Last;
uint8_t AD_Battery_i = 0;

//uint8_t TX_frequency = 0x18;			//NRF24L01����Ƶ��   6��Ƶ��ʱΪ0x18�� 8��Ƶ��ʱΪ0x5a��
//uint8_t RX_frequency = 0x5a;			//NRF24L01����Ƶ��

int16_t Vx_package = 0, Vy_package = 0, Vr_package = 0;				//�·��������ٶ�
int16_t Vx_package_last = 0, Vy_package_last = 0, Vr_package_last = 0;				//��һ֡�·��������ٶ�
uint8_t acc_set = 10;			//���ٶ����� 16ms�ںϳɼ��ٶ����ֵ����λcm/s
uint16_t acc_r_set = 60; //
uint8_t RX_Packet[25];				//�հ�
uint8_t TX_Packet[25] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5};						//�ذ�

uint16_t Received_packet = 0;
uint16_t transmitted_packet = 0;
uint8_t received_packet_flag = 0;
uint8_t	packet_flag = 0;
uint8_t	to_transmit_packet = 0;
uint16_t miss_packet = 0;			//���δ�հ���ʧ�˶��ٰ�	
uint8_t	stop_flag = 0;
uint16_t heart_toggle_flag = 0;
uint8_t ADC_circle_i = 0;	

//���ӽǶ�
double sin_angle[4] = {0.8480480961564261, -0.8480480961564261, -0.7071067811865476, 0.7071067811865476};					//double sin_angle[4] = {sin(58), -sin(58), -sin(45), sin(45)};
double cos_angle[4] = {0.5299192642332049, 0.5299192642332049, -0.7071067811865476, -0.7071067811865476};					//double cos_angle[4] = {cos(58), cos(58), -cos(45), -cos(45)};


//�������
uint8_t temp[200];				
uint8_t temp_count = 0;

//��ʱ������
uint32_t Time_count = 0;
uint32_t Time_count_last = 0;
uint32_t Interval_count = 0;
uint64_t timer_time = 0;
uint16_t Time_PID_flag = 0;
uint32_t vel_do_flag = 0;

uint16_t chipshoot_timerdelay_flag = 0;

uint16_t selftest_timer_flag = 0;

//ʱ��������λ��ms
double Interval = 0;

//����������
int Encoder_count_Motor1 = 0;
int Encoder_count_Motor2 = 0;
int Encoder_count_Motor3 = 0;
int Encoder_count_Motor4 = 0;

//�����ٶ���ʵ���ٶ�
int Vel_Motor1_aim = 0, Vel_Motor1 = 0, Vel_Now_Motor1 = 0, Vel_last_Motor1[8], Vel_last_sum_Motor1;																//ת�� ��λ��rpm
int Vel_Motor2_aim = 0, Vel_Motor2 = 0, Vel_Now_Motor2 = 0, Vel_last_Motor2[8], Vel_last_sum_Motor2;																//ת�� ��λ��rpm
int Vel_Motor3_aim = 0, Vel_Motor3 = 0, Vel_Now_Motor3 = 0, Vel_last_Motor3[8], Vel_last_sum_Motor3;																//ת�� ��λ��rpm
int Vel_Motor4_aim = 0, Vel_Motor4 = 0, Vel_Now_Motor4 = 0, Vel_last_Motor4[8], Vel_last_sum_Motor4;																//ת�� ��λ��rpm
//uint8_t vel_filter_flag = 0, vel_filter_temp;

//ռ�ձ�
double PWM_Pulse_Motor1, PWM_Pulse_Motor2, PWM_Pulse_Motor3, PWM_Pulse_Motor4, PWM_Pulse_Motord;	

//ʵ��PWM ռ��value
int PWM_Pulse_Motor1_value, PWM_Pulse_Motor2_value, PWM_Pulse_Motor3_value, PWM_Pulse_Motor4_value;

//�ٶ�ת��ϵ��
double Vel_k2 = 0.520573;                                     //double Vel_k2 = 3.18 *  4 * 360 / 2 / 3.1415926 / 2.8 / 1000 * 2;		//  cm/s  ----->>>>>>>   count/2ms

//PI����
const uint32_t Motor_KP = 110, Motor_KI = 30;

int PID_I_Limit = 266; //7995 / Motor_KI;

//PID��I������
int Motor_EK_Motor1 = 0, Motor_EK_Motor2 = 0, Motor_EK_Motor3 = 0,Motor_EK_Motor4 = 0;
int Motor_KS_Motor1 = 0, Motor_KS_Motor2 = 0, Motor_KS_Motor3 = 0,Motor_KS_Motor4 = 0;

//USART����
uint16_t USART_flag = 0, USART_flag2 = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

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
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_HRTIM_Init();
  MX_SPI1_Init();
  MX_I2C3_Init();
  MX_TIM2_Init();
  MX_TIM7_Init();
  MX_SPI2_Init();
  MX_TIM14_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  MX_TIM15_Init();
  MX_TIM16_Init();
  MX_TIM12_Init();
  MX_I2C2_Init();
  MX_UART8_Init();
  MX_I2C4_Init();
  MX_SPI3_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */

	Init_Timers();						//���������Timer��PWM��ʼ��
	Init_PCA9539();   				//PCA9539��ʼ��
	
	NRF24L01_RX_Init();				//NRF24L01���ճ�ʼ��
	NRF24L01_TX_Init();				//NRF24L01���ͳ�ʼ��
	RX_Mode();								//NRF24L01����Ϊ����ģʽ
	TX_Mode();								//NRF24L01����Ϊ����ģʽ
	HAL_TIM_Base_Start_IT(&htim5);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
////////////////	if((robot_set == 1) && (tx_mode == 0)){
////////////////		
////////////////		//��������100ms
////////////////		HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
////////////////		htim16.Instance->CCR1 = 300;
////////////////		//PWM_SET_VALUE(&htim16,TIM_CHANNEL_1,300);
////////////////		HAL_Delay(100);
////////////////		//PWM_SET_VALUE(&htim16,TIM_CHANNEL_1,0);
////////////////		htim16.Instance->CCR1 = 0;
////////////////		
////////////////		while(1){			
////////////////			Init_PCA9539();   				//PCA9539��ʼ��
////////////////			
////////////////			switch(robot_set){
////////////////				case 2 :
////////////////					if(selftest_timer_flag  > 8){
////////////////						selftest_timer_flag = 0;
////////////////						
////////////////						if(tx_mode <= 8)
////////////////							Vx_package = 10 * tx_mode;
////////////////						else
////////////////							Vx_package = 10 * (tx_mode - 17);
////////////////						motion_planner();
////////////////					}
//////////////////					Vx_package = 20;
//////////////////					motion_planner();
////////////////				break;
////////////////				
//////////////////				Vx_package = 0;
//////////////////				motion_planner();
////////////////					
////////////////				case 3 :
////////////////					Is_Infrared();				//�Ƿ񴥷�����
////////////////					if(Robot_Is_Infrared == 0){
////////////////						Robot_drib = tx_mode;
////////////////						dribber();
////////////////					}
////////////////				break;
////////////////				
//////////////////				Robot_drib = 0;
//////////////////				dribber();
////////////////					
////////////////				case 4 :
////////////////					Is_BatteryLow_BootCharged();			//��ص�ѹ�Ƿ����15.2V�������Ƿ��絽60V
////////////////					Robot_Boot_Power = 10 * tx_mode;
////////////////					Robot_Chip_Or_Shoot = 1;
////////////////					Shoot_Chip();	
////////////////				break;
////////////////				
////////////////				case 5 :
////////////////					Is_BatteryLow_BootCharged();			//��ص�ѹ�Ƿ����15.2V�������Ƿ��絽60V
////////////////					Robot_Boot_Power = 10 * tx_mode;
////////////////					Robot_Chip_Or_Shoot = 0;
////////////////					Shoot_Chip();	
////////////////				break;
////////////////				
////////////////				case 6 :
////////////////					htim12.Instance->CCR1 = 10 * tx_mode * 500;
////////////////					HAL_Delay(2);
////////////////				break;	
////////////////			}
////////////////		}
////////////////	}
  while(1)
  {
		
//		//�������
//		sprintf(temp, "%d", AD_Battery_Last);	
//		HAL_UART_Transmit(&huart3, &robot_num, 1, 0xffff);
		
////		//����2401 check
////		if(NRF24L01_TX_Check() == 0){
////			HAL_GPIO_TogglePin(TX_COM_GPIO_Port, TX_COM_Pin);
////			HAL_Delay(100);
////		}
////		if(NRF24L01_RX_Check() == 0){
////			HAL_GPIO_TogglePin(RX_COM_GPIO_Port, RX_COM_Pin);
////			HAL_Delay(100);
////		}
////		
		
		if(Received_packet == 50){
			HAL_GPIO_TogglePin(RX_COM_GPIO_Port, RX_COM_Pin);
			Received_packet = 0;
		};
		if(transmitted_packet == 10){
			HAL_GPIO_TogglePin(TX_COM_GPIO_Port, TX_COM_Pin);
			transmitted_packet = 0;
		}

		Is_Infrared();				//�Ƿ񴥷�����
		dribber();
		Is_BatteryLow_BootCharged();			//��ص�ѹ�Ƿ����15.2V�������Ƿ��絽60V
		Shoot_Chip();					//ƽ����
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable 
  */
  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);
  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) 
  {
    
  }
  /** Macro to configure the PLL clock source 
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 4;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_HRTIM1|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_UART8|RCC_PERIPHCLK_SPI3
                              |RCC_PERIPHCLK_SPI1|RCC_PERIPHCLK_SPI2
                              |RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_I2C3
                              |RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_I2C4
                              |RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.PLL2.PLL2M = 1;
  PeriphClkInitStruct.PLL2.PLL2N = 19;
  PeriphClkInitStruct.PLL2.PLL2P = 2;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_D3PCLK1;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_CLKP;
  PeriphClkInitStruct.Hrtim1ClockSelection = RCC_HRTIM1CLK_CPUCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

//ʱ���ж�
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == htim5.Instance)
	{
			//���̶������ʱ������
			Encoder_count_Motor1 = - (int16_t)(__HAL_TIM_GET_COUNTER(&htim1));
			__HAL_TIM_SET_COUNTER(&htim1,0);
			Encoder_count_Motor2 = - (int16_t)(__HAL_TIM_GET_COUNTER(&htim2));
			__HAL_TIM_SET_COUNTER(&htim2,0);
			Encoder_count_Motor3 = - (int16_t)(__HAL_TIM_GET_COUNTER(&htim3)) ;
			__HAL_TIM_SET_COUNTER(&htim3,0);
			Encoder_count_Motor4 = - (int16_t)(__HAL_TIM_GET_COUNTER(&htim4));
			__HAL_TIM_SET_COUNTER(&htim4,0);

		
			//��������ת��
			Vel_Now_Motor1 = Encoder_count_Motor1;
			Vel_Now_Motor2 = Encoder_count_Motor2;
			Vel_Now_Motor3 = Encoder_count_Motor3;
			Vel_Now_Motor4 = Encoder_count_Motor4;
			
			//PID EK��
			Motor_EK_Motor1 = Vel_Motor1 - Vel_Now_Motor1;
			Motor_EK_Motor2 = Vel_Motor2 - Vel_Now_Motor2;
			Motor_EK_Motor3 = Vel_Motor3 - Vel_Now_Motor3;
			Motor_EK_Motor4 = Vel_Motor4 - Vel_Now_Motor4;
			
			//PID I������
			Motor_KS_Motor1 += Motor_EK_Motor1;
			Motor_KS_Motor2 += Motor_EK_Motor2;
			Motor_KS_Motor3 += Motor_EK_Motor3;
			Motor_KS_Motor4 += Motor_EK_Motor4;
			
			if ((Motor_KS_Motor1)> PID_I_Limit) Motor_KS_Motor1= PID_I_Limit;
			if ((Motor_KS_Motor1)<-PID_I_Limit) Motor_KS_Motor1=-PID_I_Limit;

			if ((Motor_KS_Motor2)> PID_I_Limit) Motor_KS_Motor2= PID_I_Limit;
			if ((Motor_KS_Motor2)<-PID_I_Limit) Motor_KS_Motor2=-PID_I_Limit;

			if ((Motor_KS_Motor3)> PID_I_Limit) Motor_KS_Motor3= PID_I_Limit;
			if ((Motor_KS_Motor3)<-PID_I_Limit) Motor_KS_Motor3=-PID_I_Limit;
			
			if ((Motor_KS_Motor4)> PID_I_Limit) Motor_KS_Motor4= PID_I_Limit;
			if ((Motor_KS_Motor4)<-PID_I_Limit) Motor_KS_Motor4=-PID_I_Limit;
			
			//PID
			PWM_Pulse_Motor1_value = Motor_KP * Motor_EK_Motor1 + Motor_KI * Motor_KS_Motor1;
			PWM_Pulse_Motor2_value = Motor_KP * Motor_EK_Motor2 + Motor_KI * Motor_KS_Motor2;
			PWM_Pulse_Motor3_value = Motor_KP * Motor_EK_Motor3 + Motor_KI * Motor_KS_Motor3;
			PWM_Pulse_Motor4_value = Motor_KP * Motor_EK_Motor4 + Motor_KI * Motor_KS_Motor4;					
			
			//���ռ�ձ�����
			PWM_Pulse_Motor1_value = (PWM_Pulse_Motor1_value >= 7995) ? 7995 : PWM_Pulse_Motor1_value;
			PWM_Pulse_Motor1_value = (PWM_Pulse_Motor1_value <= -7995) ? -7995 : PWM_Pulse_Motor1_value;
			PWM_Pulse_Motor2_value = (PWM_Pulse_Motor2_value >= 7995) ? 7995 : PWM_Pulse_Motor2_value;
			PWM_Pulse_Motor2_value = (PWM_Pulse_Motor2_value <= -7995) ? -7995 : PWM_Pulse_Motor2_value;
			PWM_Pulse_Motor3_value = (PWM_Pulse_Motor3_value >= 7995) ? 7995 : PWM_Pulse_Motor3_value;
			PWM_Pulse_Motor3_value = (PWM_Pulse_Motor3_value <= -7995) ? -7995 : PWM_Pulse_Motor3_value;
			PWM_Pulse_Motor4_value = (PWM_Pulse_Motor4_value >= 7995) ? 7995 : PWM_Pulse_Motor4_value;
			PWM_Pulse_Motor4_value = (PWM_Pulse_Motor4_value <= -7995) ? -7995 : PWM_Pulse_Motor4_value;
			
			//����
			//HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = 4000 + PWM_Pulse_Motor1_value;
			if (PWM_Pulse_Motor1_value<0) {HAL_GPIO_WritePin(MOTOR1_DIR_GPIO_Port, MOTOR1_DIR_Pin, GPIO_PIN_RESET);}
				else {HAL_GPIO_WritePin(MOTOR1_DIR_GPIO_Port, MOTOR1_DIR_Pin, GPIO_PIN_SET);}
			if (abs(PWM_Pulse_Motor1_value)<=3) PWM_Pulse_Motor1_value = 3;
			HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = abs(PWM_Pulse_Motor1_value);
			
			if (PWM_Pulse_Motor2_value<0) {HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_Port, MOTOR2_DIR_Pin, GPIO_PIN_RESET);}
			  else {HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_Port, MOTOR2_DIR_Pin, GPIO_PIN_SET);}
			if (abs(PWM_Pulse_Motor2_value)<=3) PWM_Pulse_Motor2_value = 3;
			HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = abs(PWM_Pulse_Motor2_value);
			
			if (PWM_Pulse_Motor3_value<0) {HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_Port, MOTOR3_DIR_Pin, GPIO_PIN_RESET);}
				else {HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_Port, MOTOR3_DIR_Pin, GPIO_PIN_SET);}
			if (abs(PWM_Pulse_Motor3_value)<=3) PWM_Pulse_Motor3_value = 3;
			HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = abs(PWM_Pulse_Motor3_value);
			
			if (PWM_Pulse_Motor4_value<0) {HAL_GPIO_WritePin(MOTOR4_DIR_GPIO_Port, MOTOR4_DIR_Pin, GPIO_PIN_RESET);}
				else {HAL_GPIO_WritePin(MOTOR4_DIR_GPIO_Port, MOTOR4_DIR_Pin, GPIO_PIN_SET);}
			if (abs(PWM_Pulse_Motor4_value)<=3) PWM_Pulse_Motor4_value = 3;
			HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = abs(PWM_Pulse_Motor4_value);
			
			heart_toggle_flag ++;
			if(heart_toggle_flag == 1000){
				HAL_GPIO_TogglePin(HEART_GPIO_Port, HEART_Pin);
				heart_toggle_flag = 0;
			}
				
			htim14.Instance->CCR1 = drib_power;				//����
			
			if(NRF24L01_RxPacket(RX_Packet) == 0){
				unpack(RX_Packet);
				if(received_packet_flag == 1){
					motion_planner();
					Received_packet++;
					if ((Robot_Status != Last_Robot_Status) || ((Robot_Status & 0x40) == 0x40)) {
						Left_Report_Package = 5;
						Last_Robot_Status = Robot_Status;
					}
					else{
						if(Left_Report_Package >= 0)
							Left_Report_Package --;
						Last_Robot_Status = Robot_Status;
					}
					if(Left_Report_Package > 0){
						pack(TX_Packet);
						NRF24L01_TxPacket(TX_Packet);
						transmitted_packet++;
					}
					else{
						Robot_Status = 0;
						Last_Robot_Status = 0;
					}
				}
			}
			
			if(chipshoot_timerdelay_flag < 1000)
				chipshoot_timerdelay_flag++;
			
//			if(selftest_timer_flag <= 8)
//				selftest_timer_flag++;
	}
}


//����������
void Buzzer_Ring(){							//������PWMͨ���򿪣�Ƶ��2K/0.6��ռ�ձ�0.5
	htim16.Instance->CCR1=300;
};

//�������ر�
void Buzzer_Off(){							//������PWMͨ���򿪣�Ƶ��2K/0.6��ռ�ձ�0
	htim16.Instance->CCR1=0;
};

//PCA9539��ʼ������
void Init_PCA9539(){
	uint8_t i;
	uint8_t PCA9539_REG67_test[2] = {0xa5, 0xa5};
	uint8_t PCA9539_REG67[2] = {0xff, 0xff};
	uint8_t Pca9539_Read[2];
	uint8_t Pca9539_convert[16] = {15, 7, 11, 3, 13, 5, 9, 1, 14, 6, 10, 2, 12, 4, 8, 0};
	HAL_I2C_Mem_Read(&hi2c3, 0xe8 + (0x03<<1), 0, I2C_MEMADD_SIZE_8BIT, Pca9539_Read, 2, 0x10);
	tx_mode = Pca9539_convert[(Pca9539_Read[0] & 0x0f)];
	rx_freq = Pca9539_convert[(Pca9539_Read[0] >> 4)];
	if(tx_mode < 0x08){
		tx_freq = 4 * tx_mode;
	}
	else{
		tx_freq = 58 + 4 * tx_mode;
	}
	if(rx_freq < 0x08){
		rx_freq = 4 * rx_freq;
	}
	else{
		rx_freq = 58 + 4 * rx_freq;
	}
	robot_num = Pca9539_convert[(Pca9539_Read[1] & 0x0f)] - 1;
	robot_set = Pca9539_convert[(Pca9539_Read[1] >> 4)];
}

//�����Ƿ񴥷�
void Is_Infrared(){
	if(HAL_GPIO_ReadPin(INFRAIN_GPIO_Port, INFRAIN_Pin) == 0){
		HAL_GPIO_WritePin(INFRA_LED_GPIO_Port, INFRA_LED_Pin, GPIO_PIN_SET);
		Robot_Is_Infrared = 0;
		Robot_Status = Robot_Status & 0x30;
	}
	else{
		HAL_GPIO_WritePin(INFRA_LED_GPIO_Port, INFRA_LED_Pin, GPIO_PIN_RESET);
		Robot_Is_Infrared = 1;
		Robot_Status = Robot_Status | (1 << 6);
	}
}

//ADC�ɼ�
//��ص�ѹ�Ƿ����15.2V�������Ƿ��絽60V
//Battery_Voltage  ��ѹ��22/3.3
//Boot_Cap_Voltage ��ѹ��200/3.9
void Is_BatteryLow_BootCharged(){
	//SCB_InvalidateDCache_by_Addr ((uint32_t *)ADC_value, 2);
	AD_Battery = AD_Battery + ADC_value[0];
	AD_Boot_Cap = AD_Boot_Cap + ADC_value[1];
	
	if(AD_Battery_i >= 5){
		AD_Battery_Last = (7 * AD_Battery + AD_Battery_Last) >> 3;
		AD_Boot_Cap_Last = AD_Boot_Cap;
		AD_Battery = AD_Boot_Cap = 0;
		AD_Battery_i = 0;
	}
	AD_Battery_i ++;
	
	//��ص�ѹ����15.2V��196864 = 15.2/25.3*3.3/3.3*65535*5
	//��ص�ѹ����3.3V��42740 = 3.3/25.3*3.3/3.3*65535*5
	if((AD_Battery_Last > 122740)&&(AD_Battery_Last < 189600))
		Buzzer_Ring();
	else
		Buzzer_Off();
		
	
	//������ѹ��60V��113953 = 60/203.9*3.9/3.3*65535*5
	if(AD_Boot_Cap_Last > 73953){
		Robot_Is_Boot_charged = 1;
		HAL_GPIO_WritePin(BOOT_DONE_GPIO_Port, BOOT_DONE_Pin, GPIO_PIN_RESET);
	}
	else{
		Robot_Is_Boot_charged = 0;
		HAL_GPIO_WritePin(BOOT_DONE_GPIO_Port, BOOT_DONE_Pin, GPIO_PIN_SET);
	}
}

//����
void dribber(void){
	drib_power = drib_power_set[Robot_drib];
}

//ƽ����
void Shoot_Chip(){
	if((Robot_Boot_Power > 0) && (Robot_Is_Boot_charged == 1) && (Robot_Is_Infrared == 1) && (chipshoot_timerdelay_flag >= 1000)){
		if(Robot_Chip_Or_Shoot == 1){
				Robot_Is_Chip = 1;
				//PWM_SET_VALUE(&htim12,TIM_CHANNEL_1,);
				htim12.Instance->CCR1 = 65535 - Robot_Boot_Power * 500;
				HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);
				Robot_Status = Robot_Status | (1 << 4);
		}
		else{
				Robot_Is_Chip = 0;
//				Robot_Status = Robot_Status & 0xe0;
		}

		if(Robot_Chip_Or_Shoot == 0){
				Robot_Is_Shoot = 1;
				//PWM_SET_VALUE(&htim12,TIM_CHANNEL_2,Robot_Boot_Power * 500);
				htim12.Instance->CCR2 = 65535 - Robot_Boot_Power * 50;
				HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
				Robot_Status = Robot_Status | (1 << 5);
		}
		else{
			Robot_Is_Shoot = 0;
//			Robot_Status = Robot_Status & 0xd0;
		}
		
		Robot_Boot_Power = 0;
		
		chipshoot_timerdelay_flag  = 0;
	}
}

//���������Timer��PWM��ʼ��
void Init_Timers(){
	//Motor1��Ӧ��Timer�򿪼�PWM����
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = 3;
	HAL_HRTIM_WaveformOutputStart(&hhrtim, HRTIM_OUTPUT_TA1);//ͨ����
	HAL_HRTIM_WaveformCounterStart(&hhrtim, HRTIM_TIMERID_TIMER_A);//������ʱ��
	HAL_GPIO_WritePin(MOTOR1_RESET_GPIO_Port, MOTOR1_RESET_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR1_MODE_GPIO_Port, MOTOR1_MODE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR1_DIR_GPIO_Port, MOTOR1_DIR_Pin, GPIO_PIN_SET);
	
	
	//Motor2��Ӧ��Timer�򿪼�PWM����
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CMP1xR = 3;
	HAL_HRTIM_WaveformOutputStart(&hhrtim, HRTIM_OUTPUT_TC2);//ͨ����
	HAL_HRTIM_WaveformCounterStart(&hhrtim, HRTIM_TIMERID_TIMER_C);//������ʱ��
	HAL_GPIO_WritePin(MOTOR2_RESET_GPIO_Port, MOTOR2_RESET_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR2_MODE_GPIO_Port, MOTOR2_MODE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR2_DIR_GPIO_Port, MOTOR2_DIR_Pin, GPIO_PIN_SET);
	
	//Motor3��Ӧ��Timer�򿪼�PWM����
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CMP1xR = 3;
	HAL_HRTIM_WaveformOutputStart(&hhrtim, HRTIM_OUTPUT_TD2);//ͨ����
	HAL_HRTIM_WaveformCounterStart(&hhrtim, HRTIM_TIMERID_TIMER_D);//������ʱ��
	HAL_GPIO_WritePin(MOTOR3_RESET_GPIO_Port, MOTOR3_RESET_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR3_MODE_GPIO_Port, MOTOR3_MODE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR3_DIR_GPIO_Port, MOTOR3_DIR_Pin, GPIO_PIN_SET);
	
	//Motor4��Ӧ��Timer�򿪼�PWM����
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = 3;
	HAL_HRTIM_WaveformOutputStart(&hhrtim, HRTIM_OUTPUT_TB1);//ͨ����
	HAL_HRTIM_WaveformCounterStart(&hhrtim, HRTIM_TIMERID_TIMER_B);//������ʱ��
	HAL_GPIO_WritePin(MOTOR4_RESET_GPIO_Port, MOTOR4_RESET_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR4_MODE_GPIO_Port, MOTOR4_MODE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTOR4_DIR_GPIO_Port, MOTOR4_DIR_Pin, GPIO_PIN_SET);
	
	//MotorD��Ӧ��Timer�򿪼�PWM����						
	//PWM_SET_VALUE(,TIM_CHANNEL_1,2000);						//ͨ����
	HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
	htim14.Instance->CCR1 = 0;
	HAL_GPIO_WritePin(MOTORD_RESET_GPIO_Port, MOTORD_RESET_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTORD_MODE_GPIO_Port, MOTORD_MODE_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(MOTORD_DIR_GPIO_Port, MOTORD_DIR_Pin, GPIO_PIN_SET);
	
	//4·���Encoder��Ӧ��Timer��
	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);	
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
	
	//4·�����Ӧ��Timer��ʱ��
	
	
	//�����Ӧ��Timer��
	HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);							//����PWMͨ���򿪣�Ƶ��38K��ռ�ձ�0.2
	
	//��������100ms
	HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
	htim16.Instance->CCR1 = 300;
	//PWM_SET_VALUE(&htim16,TIM_CHANNEL_1,300);
	HAL_Delay(100);
	//PWM_SET_VALUE(&htim16,TIM_CHANNEL_1,0);
	htim16.Instance->CCR1 = 0;
	
	//����ADCת��
//	memset(ADC_value, 0, sizeof(ADC_value));
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&ADC_value, 32);
}

////////////����ռ�ձ�PWM
//////////void PWM_SET_VALUE(TIM_HandleTypeDef *htim,uint32_t Channel,uint16_t value)
//////////{
//////////	TIM_OC_InitTypeDef sConfigOC;
//////////	uint32_t pluse = value;
//////////	sConfigOC.OCMode = TIM_OCMODE_PWM1;
//////////	sConfigOC.Pulse = pluse;
//////////	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//////////	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//////////	if(htim == &htim16){
//////////		sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
//////////		sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
//////////	}
//////////	HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, Channel);
//////////	HAL_TIM_PWM_Start(htim, Channel);   
//////////}

//�������ÿ�����ӵ��ٶ�
void unpack(uint8_t *Packet){
	if((Packet[0] & 0xf0) == 0x40){
		if(robot_num == (Packet[1] & 0x0f) && (Packet[0] & 0x08)){
			received_packet_flag = 1;
			Vx_package = (Packet[2] & 0x7f) + ((Packet[17] & 0xc0) << 1);
			Vx_package = (Packet[2] & 0x80) ? ( -Vx_package ) : Vx_package;
			Vy_package = (Packet[3] & 0x7f) + ((Packet[17] & 0x30) << 3);
			Vy_package = (Packet[3] & 0x80) ? ( -Vy_package ) : Vy_package;
			Vr_package = (Packet[4] & 0x7f) + ((Packet[17] & 0x0f) << 7);
			Vr_package = (Packet[4] & 0x80) ? ( -Vr_package ) : Vr_package;
//			Robot_Is_Report = Packet[1] >> 7;
			Robot_drib = (Packet[1] >> 4) & 0x03;
			Robot_Chip_Or_Shoot = ( Packet[1] >> 6 ) & 0x01;
			Robot_Boot_Power = Packet[21] & 0x7f;
		}
		else if(robot_num == (Packet[5] & 0x0f) && (Packet[0] & 0x04)){
			received_packet_flag = 1;
			Vx_package = (Packet[6] & 0x7f) + ((Packet[18] & 0xc0) << 1);
			Vx_package = (Packet[6] & 0x80) ? ( -Vx_package ) : Vx_package;
			Vy_package = (Packet[7] & 0x7f) + ((Packet[18] & 0x30) << 3);
			Vy_package = (Packet[7] & 0x80) ? ( -Vy_package ) : Vy_package;
			Vr_package = (Packet[8] & 0x7f) + ((Packet[18] & 0x0f) << 7);
			Vr_package = (Packet[8] & 0x80) ? ( -Vr_package ) : Vr_package;
//			Robot_Is_Report = Packet[5] >> 7;
			Robot_drib = (Packet[5] >> 4) & 0x03;
			Robot_Chip_Or_Shoot = ( Packet[5] >> 6 ) & 0x01;
			Robot_Boot_Power = Packet[22] & 0x7f;
		}
		else if(robot_num == (Packet[9] & 0x0f) && (Packet[0] & 0x02)){
			received_packet_flag = 1;
			Vx_package = (Packet[10] & 0x7f) + ((Packet[19] & 0xc0) << 1);
			Vx_package = (Packet[10] & 0x80) ? ( -Vx_package ) : Vx_package;
			Vy_package = (Packet[11] & 0x7f) + ((Packet[19] & 0x30) << 3);
			Vy_package = (Packet[11] & 0x80) ? ( -Vy_package ) : Vy_package;
			Vr_package = (Packet[12] & 0x7f) + ((Packet[19] & 0x0f) << 7);
			Vr_package = (Packet[12] & 0x80) ? ( -Vr_package ) : Vr_package;
//			Robot_Is_Report = Packet[9] >> 7;
			Robot_drib = (Packet[9] >> 4) & 0x03;
			Robot_Chip_Or_Shoot = ( Packet[9] >> 6 ) & 0x01;
			Robot_Boot_Power = Packet[23] & 0x7f;
		}
		else if(robot_num == (Packet[13] & 0x0f) && (Packet[0] & 0x01)){
			received_packet_flag = 1;
			Vx_package = (Packet[14] & 0x7f) + ((Packet[20] & 0xc0) << 1);
			Vx_package = (Packet[14] & 0x80) ? ( -Vx_package ) : Vx_package;
			Vy_package = (Packet[15] & 0x7f) + ((Packet[20] & 0x30) << 3);
			Vy_package = (Packet[15] & 0x80) ? ( -Vy_package ) : Vy_package;
			Vr_package = (Packet[16] & 0x7f) + ((Packet[20] & 0x0f) << 7);
			Vr_package = (Packet[16] & 0x80) ? ( -Vr_package ) : Vr_package;
//			Robot_Is_Report = Packet[13] >> 7;
			Robot_drib = (Packet[13] >> 4) & 0x03;                                      
			Robot_Chip_Or_Shoot = ( Packet[13] >> 6 ) & 0x01;
			Robot_Boot_Power = Packet[24] & 0x7f;
		}
	}
		else
			received_packet_flag = 0;
};

void pack(uint8_t *TX_Packet){
	memset(TX_Packet, 0xa5, 25);
	TX_Packet[0] = 0xff;
	TX_Packet[1] = 0x02;
	TX_Packet[2] = robot_num + 1;
	TX_Packet[3] = Robot_Status;
}

void motion_planner(){
	int16_t acc_x = 0;
	int16_t acc_y = 0;
	double acc_whole = 0;
	double sin_x = 0;
	double sin_y = 0;
	acc_x = Vx_package - Vx_package_last;
	acc_y = Vy_package - Vy_package_last; 
	acc_whole = acc_x * acc_x + acc_y * acc_y ;
	acc_whole = sqrt(acc_whole);
	sin_x = acc_x / acc_whole;
	sin_y = acc_y / acc_whole;

	if (acc_whole > acc_set)
	{
		acc_whole = acc_set;
		acc_x = acc_whole * sin_x;
		acc_y = acc_whole * sin_y;
		Vx_package = Vx_package_last + acc_x;
		Vy_package = Vy_package_last + acc_y; 
	}
	
	if((Vr_package - Vr_package_last)  >  acc_r_set) Vr_package = Vr_package_last + acc_r_set;
	if((Vr_package - Vr_package_last)  < -acc_r_set) Vr_package = Vr_package_last - acc_r_set;
	
	Vx_package_last = Vx_package;
	Vy_package_last = Vy_package;
	Vr_package_last = Vr_package;
	
	Vel_Motor1 = (sin_angle[0] * Vx_package + cos_angle[0] * Vy_package + 0.025 * 8.2 * Vr_package) * Vel_k2;
	Vel_Motor2 = (sin_angle[1] * Vx_package + cos_angle[1] * Vy_package + 0.025 * 8.2 * Vr_package) * Vel_k2;
	Vel_Motor3 = (sin_angle[2] * Vx_package + cos_angle[2] * Vy_package + 0.025 * 8.2 * Vr_package) * Vel_k2;
	Vel_Motor4 = (sin_angle[3] * Vx_package + cos_angle[3] * Vy_package + 0.025 * 8.2 * Vr_package) * Vel_k2;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/