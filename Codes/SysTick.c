/*Functions definitions of Systick header file */

/*------------------------------*/
#include "SysTick.h"
#include "tm4c123gh6pm.h"


//4)----------- system clock -----------
void SysTick_Init(void){
	NVIC_ST_CTRL_R = 0;              //disable systick during setup
	NVIC_ST_CTRL_R = 0x00000005;     //enable core clock.
}

void SysTickWait(unsigned long delay){
	NVIC_ST_RELOAD_R = delay - 1;
	NVIC_ST_CURRENT_R = 0;
	while((NVIC_ST_CTRL_R&0x00010000) == 0){
	}  //wait for the count flag to be 0
}
//800000*12.5ns = 10ms 
//12.5ns = 1 / 80*10^6 Hz
void SysTick(unsigned long delay){
	unsigned long i;
	for(i=0; i < delay; i++){
		SysTickWait(800000); //wait 10ms
	}
}
