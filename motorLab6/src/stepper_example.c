/*******************************************************************************
  * File Name          : stepper_example.c
  * Description        : This program allows user to controll the Speed, Steps and Rotation degree of the stepper motor. 
                       : It utilizes the NS and MS timer function from the timer lab that uses TIM11 without interrupts.
                       : D6 -> STEP, D5 -> FR and A2 -> VREF  
  * Author             : Harsh Hirenbhai Shah
                       : Arsh Sandhu
  * Date               : Oct-13-2021
  ******************************************************************************
  */

#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "main.h"



//tim11 handel
TIM_HandleTypeDef htim11;
int isStepperEnabled = -1;

/**
  * @brief Nano second delay function.
  * @param uint16_t time
  * @retval None
  */
void DelayNS (uint16_t us)
{
	TIM11->CNT = 0;
	while (TIM11->CNT < us);
}
/**
  * @brief Microseconds second delay function.
  * @param uint16_t time
  * @retval None
  */
void DelayMS (uint16_t ms)
{
	for (uint16_t i=0; i<ms; i++)
	{
		DelayNS (1000); // 1 ms = 1000 ns.
	}
}


/**
  * @brief TIM11 Initialization Function
  * @param None
  * @retval None
  */
void timerInit() {
  htim11.Instance = TIM11;
  htim11.Init.Prescaler = 80;
  htim11.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim11.Init.Period = 65535;
  htim11.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim11.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim11) != HAL_OK)
  {
    printf("could not initialize the timer.\n");
  }

  HAL_TIM_Base_Start(&htim11);
}

/**
  * @brief GPIO Initialization Function for stepper motors
  * @param None
  * @retval None
  */
void stepperInit() {
   GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10|GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 LD2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_4|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
} 

/**
  * @brief Moter event function
  * @param int mode of the event, does it require interaction?
  * @retval ParserReturnVal_t enum. 
  */
ParserReturnVal_t motorEvent(int mode)
{
  stepperInit();
  timerInit();
  

  if(mode != CMD_INTERACTIVE) return CmdReturnOk;

if(isStepperEnabled == 0)
{
  printf("Please enable the stepper drive first. See help for more details.\n");
  return CmdReturnOk;
} 

 
uint32_t delay, rc, step = 0,rcTwo, isCW, rcisCW;
  
  // Delay Input    
  rc=fetch_uint32_arg(&delay);
  if(rc)
  {
    printf("Delay must be provided.\n");
    return CmdReturnBadParameter1;
  }

  //Step Input
  rcTwo = fetch_uint32_arg(&step);
  if(rcTwo)
  {
    step = 400;
    printf("User did not provided steps, default steps are 400.\n");
  }

  //Rotation Input
  rcisCW = fetch_uint32_arg(&isCW);
  if(rcisCW)
  {
    isCW = 1;
    printf("User did not provide the direction, so running in CW.\n");
  }  

  //Telling motor to rotate CW or CCW
  if(isCW == 1)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  } 
  else if(isCW == 0)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  }

  //Checking if we need to switch on the Motor.
  if(isStepperEnabled == 1)
  {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);  
  } else {
    printf("Added another check if we need to turn on the moter. \n");
    return CmdReturnOk;
  }

  //Rotating the motor
	  for(int x = 0; x<step; x++)
	  {
      //printf("inside for loop %d\n", x);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
		  DelayNS(delay);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
		  DelayNS(delay);
      WDTFeed();
	  }

  //WDTFeed();
  return CmdReturnOk;
}

/**
  * @brief Timer11 elpased callback.
  * @param TIM_HandleTypeDef timer
  * @retval void enum. 
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  // Check which version of the timer triggered this callback and toggle LED
  if (htim == &htim11 )
  { 
    //uint32_t timer_val; 
    __HAL_TIM_SET_COUNTER(&htim11, 0);
    //timer_val = __HAL_TIM_GET_COUNTER(&htim11);
    //printf("Ticks - In callback: %ld\n", timer_val);
    WDTFeed();
    //TIM11->CNT = 0;
  }
}

ADD_CMD("step",motorEvent,"                step <delay> <step> <direction> 0=CW, 1=CCW");

/**
  * @brief enable Disable stepper motor event.
  * @param int mode, do we need to run this function interactively?
  * @retval ParserReturnVal_t return code. 
  */
ParserReturnVal_t  enableDisableStepper(int mode)
{
  if(mode != CMD_INTERACTIVE) return CmdReturnOk;
  
  //stepper and timer init
  stepperInit();
  timerInit();

  uint32_t stepperStatus, rc;

  //taking the input from user for enabling and disabling the stepper motor. 
  rc=fetch_uint32_arg(&stepperStatus);
  if(rc)
  {
    printf("Please enable or disable stepper controller.\n");
    return CmdReturnBadParameter1;
  }

  //validating against the user input and reflecting it to the global variable.
  if(stepperStatus == 1)
  {   
    isStepperEnabled = 1; 
    printf("stepper motor enabled.\n");
  } else if (stepperStatus == 0)
  {
    isStepperEnabled = 0; 
    printf("stepper motor disabled.\n");
  }
  
  
return CmdReturnOk;
}

ADD_CMD("se",enableDisableStepper,"                stepperenable <0|1>");

/**
  * @brief runs the motor in trapozoid profile
  * @param int mode, do we need to run this function interactively?
  * @retval ParserReturnVal_t return code. 
  */
ParserReturnVal_t  trapoStepperRunner(int mode)
{
  if(mode != CMD_INTERACTIVE) return CmdReturnOk;
  stepperInit();
  timerInit();
  uint32_t stepTime, rc, directon, directionRc, rotationCount, rotationCountRc, delayOne, delayOneRc, delayTwo, delayTwoRc;
   
  //User input for step time.
  rc=fetch_uint32_arg(&stepTime);
  if(rc)
  {
    printf("please provide step time.\n");
    return CmdReturnBadParameter1;
  }

  //User input for first and last delay.
  delayOneRc=fetch_uint32_arg(&delayOne);
  if(delayOneRc)
  {
    printf("Please provid initial and final rotatio speed. \n");
    return CmdReturnBadParameter1;
  }
  
  //User input for rotation speed. 
  delayTwoRc=fetch_uint32_arg(&delayTwo);
  if(delayTwoRc)
  {
    printf("Please provid delay two.\n");
    return CmdReturnBadParameter1;
  }

  //User inpit for direction.
  directionRc=fetch_uint32_arg(&directon);
  if(directionRc)
  {
    printf("Please provide direction.\n");
    return CmdReturnBadParameter1;
  }

  //User input for how many time this trapozid cycle should be repetade.
  rotationCountRc=fetch_uint32_arg(&rotationCount);
  if(rotationCountRc)
  {
    printf("Please provide motor rotation Count.\n");
    return CmdReturnBadParameter1;
  }

  if(directon == 1)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  } else if(directon == 0)
  {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
  }
  if(isStepperEnabled == 1)
  {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);  
  }

//rotation cycle.
for(int i = 0; i < rotationCount; i++)
{
    //starting of slope.
  	for(int x = 0; x<stepTime; x++)
	  {
      //printf("inside for loop %d\n", x);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
		  DelayNS(delayOne);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
		  DelayNS(delayOne);
      WDTFeed();
	  }

    //middle part.
    for(int x = 0; x<stepTime; x++)
	  {
      //printf("inside for loop %d\n", x);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
		  DelayNS(delayTwo);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
		  DelayNS(delayTwo);
      WDTFeed();
	  }

    //ending of slope.
    for(int x = 0; x<stepTime; x++)
	  {
      //printf("inside for loop %d\n", x);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
		  DelayNS(delayOne);
		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
		  DelayNS(delayOne);
      WDTFeed();
	  }



} 


return CmdReturnOk;
}

ADD_CMD("trapo",trapoStepperRunner,"                trapo <step> <delay1> <delay2> <direction> <rotationCount> CW=0, CCW=1");
