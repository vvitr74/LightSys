#include "pb_charger.h"

//uint8_t pbChargerEn=0;
//TIM_HandleTypeDef htim16;

//enum chStates{
//	Initialization=1,
//	Bulk,
//	Absorption,
//	Equalization,
//	Float
//};

//static uint8_t currentState=Initialization;

//void pbChargerFSM(void)
//{
//	switch(currentState){
//		case Initialization:
//			if(PB_BATTERY_DEFECT)
//				return;
//			break;
//		case Bulk:
//			break;
//		case Absorption:
//			break;
//		case Equalization:
//			break;
//		case Float:
//			break;
//		default:
//			break;
//	}
//}

//void MX_TIM16_Init(void)
//{
//  TIM_ClockConfigTypeDef sClockSourceConfig;
//  TIM_MasterConfigTypeDef sMasterConfig;
////  TIM_OC_InitTypeDef sConfigOC;

//  htim16.Instance = TIM16;
//  htim16.Init.Prescaler = 0xFFFF;	//65535
//  htim16.Init.Period = 0xFFFF;	
//  htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//  HAL_TIM_Base_Init(&htim16);

//  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
//  HAL_TIM_ConfigClockSource(&htim16, &sClockSourceConfig);

////  HAL_TIM_PWM_Init(&htim3);

//  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//  HAL_TIMEx_MasterConfigSynchronization(&htim16, &sMasterConfig);

////  sConfigOC.OCMode = TIM_OCMODE_PWM1;
////  sConfigOC.Pulse = 0;
////  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
////  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
////  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
//}