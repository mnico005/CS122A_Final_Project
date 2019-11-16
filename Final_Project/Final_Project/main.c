/*
 * Final_Project.c
 *
 * Created: 11/9/2019 2:17:39 AM
 * Author : Marios
 */ 

#include <avr/io.h>
#include "ADC.h"
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <timer.h>
#include <bit.h>
#include <scheduler.h>
#include <io.c>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <string.h>
#include "SPI_Master_C_file.c"
#include "Font.h"
#include "timer.h"
#include "Nokia_5110.h"
#include "Player2SM.c"

enum LocStates {LocStart, LocUpdate};

unsigned char segLoc;
unsigned char yLoc;
unsigned char xLoc;
unsigned short y;
unsigned short x;
unsigned char lastDir;
	//ADC x > 900 => JOYStick down => 0
	//ADC x < 200 => JOYStick up => 1
	//ADC y > 900 => JOYStick right => 2
	//ADC y < 200 => JOYStick left => 3
int LocTick(int state){
	switch(state){//Transitions
		case LocStart:
			state = LocUpdate;
			break;
		case LocUpdate:
			state = LocUpdate;
			break;
		default:
			state = LocStart;
			break;
	}
	switch(state){//Actions
		case LocStart:
			segLoc = 0;
			yLoc = 2;
			xLoc = 0;
			y = 0;
			x = 0;
			lastDir = 2;
			break;
		case LocUpdate:
			x = analog_read(0);
			y = analog_read(1);
			//y direction
			if (x > 900)
			{
				if(segLoc == 0 && yLoc != 5){
					segLoc = 7;
					yLoc ++;
					}else if(segLoc != 0){
					segLoc --;
				}
				lastDir = 0;
			}else if (x < 200)
			{
				if (segLoc == 8)
				{
					segLoc = 0;
					yLoc -- ;
					}else if(segLoc == 0 && yLoc != 0){
					segLoc ++;
				}else if (segLoc > 0)
				{
					segLoc ++;
				}
				lastDir = 1;
			}
			
			//x direction
			if (y > 900 && xLoc < 76)
			{
				xLoc ++;
				lastDir = 2;
			}else if (y < 200 && xLoc > 0)
			{
				xLoc --;
				lastDir = 3;
			}
			break;
		default:
			break;
	}
	return state;
}

enum BuildStates {BuildStart, BuildCharacter}; 

static unsigned char character[] = {0x00, 0x08, 0xEB, 0x3F, 0x3F, 0xEB, 0x08, 0x00};
static unsigned char empty[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char character_top1[8]; static unsigned char character_top2[8]; static unsigned char character_top3[8]; static unsigned char character_top4[8];
static unsigned char character_top5[8]; static unsigned char character_top6[8]; static unsigned char character_top7[8]; static unsigned char character_top8[8];
static unsigned char* character_top[] = {empty, character_top1, character_top2, character_top3, character_top4, character_top5,  character_top6, character_top7, character_top8};

static unsigned char character_bottom1[8];static unsigned char character_bottom2[8];static unsigned char character_bottom3[8];static unsigned char character_bottom4[8];
static unsigned char character_bottom5[8];static unsigned char character_bottom6[8];static unsigned char character_bottom7[8];static unsigned char character_bottom8[8];
static unsigned char* character_bottom[] = {character, character_bottom1, character_bottom2, character_bottom3, character_bottom4, character_bottom5, character_bottom6, character_bottom7, character_bottom8};

unsigned char j = 0;
unsigned char i = 0;
int BuildTick(int state){
	switch(state){//Transitions
		case BuildStart:
			state = BuildCharacter;
			break;
		case BuildCharacter:
			state = BuildCharacter;
			break;
		default:
			state = BuildStart;
			break;
	}
	switch(state){//Actions
		case BuildStart:
			//initialize bottom
			for (j = 1; j < 9; j++) // 9
			{
				for (i=0; i<8;i++)//13
				{
					character_bottom[j][i] = (character[i] >> j);
				}
			}
			//initialize top
			for (j = 1; j < 9; j++)
			{
				for (i=0; i<8;i++)
				{
					character_top[j][i] =  (character[i] << (8-j));
				}
			}
			break;
		case BuildCharacter:
			//build
			if(yLoc == 0){
				lcd_setXY(0x80 + xLoc, 0x40 + yLoc);
				N5110_image(character, 8);
				}else{
				lcd_setXY(0x80 + xLoc , 0x40 + yLoc);
				N5110_image(character_bottom[segLoc], 8);
				lcd_setXY(0x80 + xLoc, 0x40 + yLoc - 1 );
				N5110_image(character_top[segLoc], 8);
			}
			if (segLoc == 8)
			{
				segLoc = 0;
				yLoc --;
			}
			break;
		default:
			break;
	}
	return state;
}

enum ProjStates {ProjStart, ProjWait, ProjLeft, ProjRight};
/*static unsigned char projectile1[] = {0x01}; static unsigned char projectile2[] = {0x02}; static unsigned char projectile3[] = {0x04}; static unsigned char projectile4[] = {0x08};
static unsigned char projectile5[] = {0x10}; static unsigned char projectile6[] = {0x20}; static unsigned char projectile7[] = {0x40}; static unsigned char projectile8[] = {0x80};
static unsigned char* projectile[] = {projectile8, projectile7, projectile6, projectile5, projectile4, projectile3, projectile2, projectile1};	
static unsigned char erase[] = {0x00};*/

 char ProjX;
unsigned char ProjY;
unsigned char ProjSeg;
int ProjTick(int state){
	switch(state){//Transitions
		case ProjStart:
			state = ProjWait;
			break;
		case ProjWait:
			if (!GetBit(PINC, 0) && lastDir == 2){
				ProjX = xLoc + 8;
				ProjSeg = segLoc; //+ 5;
				ProjY = yLoc;
				/*if (ProjSeg > 7)
				{
					ProjSeg -= 7;
					ProjY --;
				}*/
				state = ProjRight;
			}
			else if (!GetBit(PINC, 0) && lastDir == 3){
				ProjX = xLoc - 1;
				ProjSeg = segLoc; //+ 5;
				ProjY = yLoc;
				/*if (ProjSeg > 7)
				{
					ProjSeg -= 7;
					ProjY --;
				}*/
				state = ProjLeft;
			}else{
				state = ProjWait;
			}
			break;
		case ProjRight:
			if(ProjX > 83){
				state = ProjWait;
			}else{
				state = ProjRight;
				ProjX ++;
			}
			break;
		case ProjLeft:
			if (ProjX == 0)
			{
				state = ProjWait;
			}
			else {
				state = ProjLeft;
				ProjX --;
			}
			break;
		default:
			state = ProjStart;
			break;
	}
	switch(state){//Actions
		case ProjStart:
			ProjX = 84;
			ProjY = 6;
			ProjSeg = 8;
			break;
		case ProjWait:
			break;
		case ProjRight:
			lcd_setXY(0x80 + ProjX -1  , 0x40 + ProjY);
			N5110_image(erase, 1);
			lcd_setXY(0x80 + ProjX , 0x40 + ProjY);
			N5110_image(projectile[ProjSeg], 1);
			break;
		case ProjLeft:
			if (ProjX == 0)
			{
				lcd_setXY(0x80 + 1, 0x40 + ProjY);
				N5110_image(erase, 1);
			}else {
				lcd_setXY(0x80 + ProjX  +1, 0x40 + ProjY);
				N5110_image(erase, 1);
				lcd_setXY(0x80 + ProjX , 0x40 + ProjY);
				N5110_image(projectile[ProjSeg], 1);
			}
			break;
		default:
			break;
	}
	return state;
}
enum CollStates {collStart, collCheck, collFound};
	
int CollTick(int state){
	switch(state){//Transitions
		case collStart:
			state = collCheck;
			break;
		case collCheck:
			if (yLoc2 == ProjY && ProjSeg >= segLoc2 && ProjX < xLoc2 +8 && ProjX > xLoc2)
			{
				state = collFound;
			}
			else if (yLoc2-1 == ProjY && ProjSeg < segLoc2 && ProjX < xLoc2 +8 && ProjX > xLoc2)
			{
				state = collFound;
			}
			else{
				state = collCheck;
			}
			break;
		case collFound:
			state = collFound;
			break;
		default:
			 state = collStart;
			 break;
	}
	switch(state){
		case collStart:
		
			break;
		case collCheck:
		
			break;
		case collFound:
			N5110_clear();
			lcd_setXY(0x80, 0x40 +2);
			N5110_Data("PLAYER 1 WINS!");
			while(1){}
			break;
			
		default:
			break;
	}
	return state;
}
/////////////////////////////////////////
enum CollStates2 {collStart2, collCheck2, collFound2};

int CollTick2(int state){
	switch(state){//Transitions
		case collStart2:
		state = collCheck2;
		break;
		case collCheck2:
		if (yLoc == ProjY2 && ProjSeg2 >= segLoc && ProjX2 < xLoc +8 && ProjX2 > xLoc)
		{
			state = collFound2;
		}
		else if (yLoc-1 == ProjY2 && ProjSeg2 < segLoc && ProjX2 < xLoc +8 && ProjX2 > xLoc)
		{
			state = collFound2;
		}
		else{
			state = collCheck2;
		}
		break;
		case collFound2:
		state = collFound2;
		break;
		default:
		state = collStart2;
		break;
	}
	switch(state){
		case collStart2:
		
		break;
		case collCheck2:
		
		break;
		case collFound2:
		N5110_clear();
		lcd_setXY(0x80, 0x40 +2);
		N5110_Data("PLAYER 2 WINS!");
		while(1){}
		break;
		
		default:
		break;
	}
	return state;
}
int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRD = 0xFF; PORTD = 0x00;
	DDRC = 0x00; PORTC = 0xFF;
	//static unsigned char bottom[]= {0x00, 0x01, 0x03, 0xFF, 0x9F, 0x1F, 0xFF, 0x8F, 0x03, 0x00, 0x01, 0x00, 0x00};
	//static unsigned char top[] =   {0xC0, 0x80, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xFF, 0xFD, 0x97, 0x97, 0x17, 0x07};
	
    ADC_init();
	SPI_Init();
	N5110_init();
	N5110_clear();
	lcd_setXY(0x9F ,0x42);
	
	unsigned long int Tick_Loc_calc = 75;
	unsigned long int Tick_Build_calc = 75;
	unsigned long int Tick_Proj_calc = 40;
	unsigned long int Tick_Loc2_calc = 75;
	unsigned long int Tick_Build2_calc = 75;
	unsigned long int Tick_Proj2_calc = 40;
	unsigned long int Tick_Coll_calc = 40;
	
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(Tick_Build_calc, Tick_Loc_calc);
	tmpGCD = findGCD(tmpGCD, Tick_Proj_calc);
	tmpGCD = findGCD(tmpGCD, Tick_Loc2_calc);
	tmpGCD = findGCD(tmpGCD, Tick_Build2_calc);
	tmpGCD = findGCD(tmpGCD, Tick_Proj2_calc);
	tmpGCD = findGCD(tmpGCD, Tick_Coll_calc);
	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;
	
	//Recalculate GCD periods for scheduler
	unsigned long int Tick_Loc_period = Tick_Loc_calc/GCD;
	unsigned long int Tick_Build_period = Tick_Build_calc/GCD;
	unsigned long int Tick_Proj_period = Tick_Proj_calc/GCD;
	unsigned long int Tick_Build2_period = Tick_Build2_calc/GCD;
	unsigned long int Tick_Loc2_period = Tick_Loc2_calc/GCD;
	unsigned long int Tick_Proj2_period = Tick_Proj2_calc/GCD;
	unsigned long int Tick_Coll_period = Tick_Coll_calc/GCD;

	
	static task task1;
	static task task2;
	static task task3;
	static task task4;
	static task task5;
	static task task6;
	static task task7;
	static task task8;
	
	task *tasks[] = {&task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	// Task 1
	task1.state = -1;//Task initial state.
	task1.period = Tick_Loc_period;//Task Period.
	task1.elapsedTime = Tick_Loc_period;//Task current elapsed time.
	task1.TickFct = &LocTick;//Function pointer for the tick.
	
	// Task 2
	task2.state = -1;//Task initial state.
	task2.period = Tick_Build_period;//Task Period.
	task2.elapsedTime = Tick_Build_period;//Task current elapsed time.
	task2.TickFct = &BuildTick;//Function pointer for the tick.
	
	// Task 3
	task3.state = -1;//Task initial state.
	task3.period = Tick_Proj_period;//Task Period.
	task3.elapsedTime = Tick_Proj_period;//Task current elapsed time.
	task3.TickFct = &ProjTick;//Function pointer for the tick.
	
	// Task 4
	task4.state = -1;//Task initial state.
	task4.period = Tick_Loc2_period;//Task Period.
	task4.elapsedTime = Tick_Loc2_period;//Task current elapsed time.
	task4.TickFct = &LocTick2;//Function pointer for the tick.
	
	// Task 5
	task5.state = -1;//Task initial state.
	task5.period = Tick_Build2_period;//Task Period.
	task5.elapsedTime = Tick_Build2_period;//Task current elapsed time.
	task5.TickFct = &BuildTick2;//Function pointer for the tick.
	
	// Task 6
	task6.state = -1;//Task initial state.
	task6.period = Tick_Proj2_period;//Task Period.
	task6.elapsedTime = Tick_Proj2_period;//Task current elapsed time.
	task6.TickFct = &ProjTick2;//Function pointer for the tick.
	
	// Task 7
	task7.state = -1;//Task initial state.
	task7.period = Tick_Coll_period;//Task Period.
	task7.elapsedTime = Tick_Coll_period;//Task current elapsed time.
	task7.TickFct = &CollTick;//Function pointer for the tick.
	
	// Task 8
	task8.state = -1;//Task initial state.
	task8.period = Tick_Coll_period;//Task Period.
	task8.elapsedTime = Tick_Coll_period;//Task current elapsed time.
	task8.TickFct = &CollTick2;//Function pointer for the tick.
	
	
	TimerSet(GCD);
	TimerOn();
	
	i=0;
    while (1) 
    {
		

		
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
				
		while(!TimerFlag);
		TimerFlag = 0;
		
		
		
    }
}

