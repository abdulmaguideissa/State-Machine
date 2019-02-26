/*SysTick.h
 *Header file for the system clock to initiate a proper and precise time delay.
 *7/3/2017
 *Author: Abdulmaguid
*/
/*-----------------------------------*/
#ifndef __SysTick_H__
#define __SysTick_H__


//1)initialization. 
void SysTick_Init(void);


//2)Loading the reload register by proper count value
void SysTick(unsigned long delay);


//3)Create the required delay
void SysTickWait(unsigned long delay);




#endif
