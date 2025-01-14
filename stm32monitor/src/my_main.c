/*******************************************************************************
  * File Name          : my_main.c
  * Description        : Allows user to controll custom commands and demonstration of a small RTOS.
                       : This src code follows the template provided in the stm32montor. 
  * Author             : Harsh Hirenbhai Shah
                       : Arsh Sandhu
  * Date               : Sep/22/2021
  ******************************************************************************
  */#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "common.h"

/* This include will give us the CubeMX generated defines */
#include "main.h"

/* This function is called from the CubeMX generated main.c, after all
 * the HAL peripherals have been initialized. */
void my_init(void)
{
  /* Initialize the terminal system */
  TerminalInit();

  /* Print out reset source */
  WDTCheckReset();
  
  /* Initialize the task system */
  TaskingInit();

  my_Init();

}

/* This function is called from inside the CubeMX generated main.c,
 * inside the while(1) loop. */
void my_main(void)
{

  TaskingRun();  /* Run all registered tasks */
  my_Loop();

  WDTFeed();
}

ParserReturnVal_t CmdLed(int mode)
{
  uint32_t val,rc;
  
  if(mode != CMD_INTERACTIVE) return CmdReturnOk;

  rc = fetch_uint32_arg(&val);
  if(rc) {
    printf("Please supply 1 | 0 to turn on or off the LED\n");
    return CmdReturnBadParameter1;
  }

  val = val ? 1 : 0;
#if defined(LD2_Pin)
  HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,val);
#elif defined(LD3_Pin)
  HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin,val);
#endif
  
  return CmdReturnOk;
}

ADD_CMD("led",CmdLed,"0 | 1           Control LED")

