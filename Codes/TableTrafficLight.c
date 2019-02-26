// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Daniel Valvano, Jonathan Valvano
// January 15, 2016
//Author: Abdulmaguid Eissa.

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "SysTick.h"

// ***** 2. Global Declarations Section *****
//FSM design and initialization
//1)----------- defining structure ------------
struct State{
	unsigned long PBout;
  unsigned long	PFout;
	unsigned long time;
	const struct State *Next[8];
};
typedef const struct State sType;
//2)-------- possible states -----------.
#define GoWest  &fsm[0]
#define Wait_W  &fsm[1]
#define GoSouth &fsm[2]
#define Wait_S  &fsm[3]
#define Walk    &fsm[4]
#define D_Walk  &fsm[5]
#define OFF     &fsm[6]
#define AllRed  &fsm[7]
#define OffRed  &fsm[8]
//3) ---------- initilization ----------
sType fsm[9] = {
	{0x0C, 0x02, 50, GoWest, GoWest, Wait_W, Wait_W, Wait_W, Wait_W, Wait_W, Wait_W},      //state 1 
	{0x14, 0x02, 50, GoSouth, GoSouth, GoSouth, GoSouth, Walk, Walk, Walk, GoSouth},       //state 2
  {0x21, 0x02, 50, GoSouth, Wait_S, GoSouth, Wait_S, Wait_S, Wait_S, Wait_S, Wait_S},    //state 3 
  {0x22, 0x02, 50, GoWest, GoWest, GoWest, GoWest, Walk, Walk, Walk, Walk},              //state 4
  {0x24, 0x08, 50, Walk, OFF, OFF, OFF, Walk, OFF, OFF, D_Walk},                         //state 5
  {0x24, 0x02, 50, D_Walk, GoWest, GoSouth, GoWest, Walk, Walk, GoSouth, OFF},           //state 6 
  {0x24, 0x00, 50, OFF, GoWest, GoSouth, GoSouth, Walk, GoWest, GoSouth, AllRed},        //state 7 
  {0x24, 0x02, 50, AllRed, GoWest, GoSouth, GoSouth, Walk, GoWest, Walk, OffRed},        //state 8
  {0x24, 0x00, 50, OffRed, GoWest, GoSouth, GoWest, Walk, Walk, Walk, GoWest}		         //state 9 
};

/* --------- Globals ------------*/
#define SENSOR (*((volatile unsigned long *) 0x4002401C))  //PE0-2
#define PFOut  (*((volatile unsigned long *) 0x40025028))  //PF1,3
#define PBOut  (*((volatile unsigned long *) 0x400050FC))  //PB0-5
unsigned long Input; 
unsigned long CState;   //current state.
const struct State *Pt;
// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortB_Init(void);        //intiallize port B
void PortE_Init(void);        //intiallize port E
void PortF_Init(void);        //intiallize port F

//void PLL_Init(void);

// ***** 3. Subroutines Section *****


/*------------- Main -------------*/
int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
  EnableInterrupts();
	PortB_Init();
	PortE_Init();
	PortF_Init();
	PLL_Init();
	SysTick_Init();
	Pt = &fsm[0];    //starting state.
	//CState = GoWest;
  while(1){
     PBOut = Pt->PBout;           //1. output to the cars firstly.
		 PFOut = Pt->PFout;           //then to the walkers.
		 SysTick(Pt->time);           //2. delay amount of time (0.5 s).
		 Input = SENSOR;              //3. read the input from sensors.
		 Pt = Pt->Next[Input];        //4. update the current state as function of current state and input."Moore Machine"
  }
}

/* --------- initializations for ports, PLL and system clock ------------ */
//1) ------- Port B ------------
void PortB_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x02;  // 1) B clock
  delay = SYSCTL_RCGC2_R;            // delay to stablize the register.
	GPIO_PORTB_DIR_R  = 0x3F;         //PB0-5 is output
 	GPIO_PORTB_AMSEL_R = 0x00;       //Disable analog functions 
  GPIO_PORTB_AFSEL_R = 0x00;	     //Disable alternate functions
	GPIO_PORTB_PCTL_R = 0x00000000;  
	GPIO_PORTB_DEN_R = 0x3F;         //Enable digital functions.
}

//2)---------- Port E -----------
void PortE_Init(void){ volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;
	delay = SYSCTL_RCGC2_R;
	GPIO_PORTE_AMSEL_R = 0x00;
	GPIO_PORTE_AFSEL_R = 0x00;
	GPIO_PORTE_PCTL_R = 0x00000000;
	GPIO_PORTE_DIR_R = 0xF0;  //PE0-2 inputs
	GPIO_PORTE_DEN_R = 0x07;  //digital enable PE0-2
}

//3)----------- Port F -----------
void PortF_Init(void){volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x20;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
	GPIO_PORTF_LOCK_R = 0x4C4F434B;   //unlock Port F
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0A;          // 5) PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_DEN_R = 0x0A;          // 7) enable digital I/O on PF3,1
}


//5)------------- PLL --------------
/*void PLL_Init(void){
	// 0) Use RCC2
  SYSCTL_RCC2_R |=  0x80000000;  // USERCC2
  // 1) bypass PLL while initializing
  SYSCTL_RCC2_R |=  0x00000800;  // BYPASS2, PLL bypass
  // 2) select the crystal value and oscillator source
  SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0)   // clear XTAL field, bits 10-6
                 + 0x00000540;   // 10101, configure for 16 MHz crystal
  SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
  // 3) activate PLL by clearing PWRDN
  SYSCTL_RCC2_R &= ~0x00002000;
  // 4) set the desired system divider
  SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL
  SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                  + (4<<22);      // configure for 80 MHz clock
  // 5) wait for the PLL to lock by polling PLLLRIS
  while((SYSCTL_RIS_R&0x00000040)==0){};  // wait for PLLRIS bit
  // 6) enable use of PLL by clearing BYPASS
  SYSCTL_RCC2_R &= ~0x00000800;
}*/
