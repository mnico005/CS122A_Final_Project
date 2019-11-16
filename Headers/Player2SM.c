enum LocStates2 {LocStart2, LocUpdate2};

unsigned char segLoc2;
unsigned char yLoc2;
unsigned char xLoc2;
unsigned short y2;
unsigned short x2;
unsigned char lastDir2;
//ADC x > 900 => JOYStick down => 0
//ADC x < 200 => JOYStick up => 1
//ADC y > 900 => JOYStick right => 2
//ADC y < 200 => JOYStick left => 3
int LocTick2(int state){
	switch(state){//Transitions
		case LocStart2:
		state = LocUpdate2;
		break;
		case LocUpdate2:
		state = LocUpdate2;
		break;
		default:
		state = LocStart2;
		break;
	}
	switch(state){//Actions
		case LocStart2:
		segLoc2 = 0;
		yLoc2 = 2;
		xLoc2 = 75;
		y2 = 0;
		x2 = 0;
		lastDir2 = 3;
		break;
		case LocUpdate2:
		x2 = analog_read(2);
		y2 = analog_read(3);
		//y direction
		if (x2 > 900)
		{
			if(segLoc2 == 0 && yLoc2 != 5){
				segLoc2 = 7;
				yLoc2 ++;
				}else if(segLoc2 != 0){
				segLoc2 --;
			}
			lastDir2 = 0;
		}else if (x2 < 200)
		{
			if (segLoc2 == 8)
			{
				segLoc2 = 0;
				yLoc2 -- ;
				}else if(segLoc2 == 0 && yLoc2 != 0){
				segLoc2 ++;
			}else if (segLoc2 > 0)
			{
				segLoc2 ++;
			}
			lastDir2 = 1;
		}
		
		//x direction
		if (y2 > 900 && xLoc2 < 76)
		{
			xLoc2 ++;
			lastDir2 = 2;
		}else if (y2 < 200 && xLoc2 > 0)
		{
			xLoc2 --;
			lastDir2 = 3;
		}
		break;
		default:
		break;
	}
	return state;
}

//////////////////////
enum BuildStates2 {BuildStart2, BuildCharacter2};

static unsigned char character2[] = {0x00, 0x08, 0xEB, 0x3F, 0x3F, 0xEB, 0x08, 0x00};
static unsigned char empty2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char character_top12[8]; static unsigned char character_top22[8]; static unsigned char character_top32[8]; static unsigned char character_top42[8];
static unsigned char character_top52[8]; static unsigned char character_top62[8]; static unsigned char character_top72[8]; static unsigned char character_top82[8];
static unsigned char* character2_top[] = {empty2, character_top12, character_top22, character_top32, character_top42, character_top52,  character_top62, character_top72, character_top82};

static unsigned char character_bottom12[8];static unsigned char character_bottom22[8];static unsigned char character_bottom32[8];static unsigned char character_bottom42[8];
static unsigned char character_bottom52[8];static unsigned char character_bottom62[8];static unsigned char character_bottom72[8];static unsigned char character_bottom82[8];
static unsigned char* character2_bottom[] = {character2, character_bottom12, character_bottom22, character_bottom32, character_bottom42, character_bottom52, character_bottom62, character_bottom72, character_bottom82};

unsigned char j2 = 0;
unsigned char i2 = 0;
int BuildTick2(int state){
	switch(state){//Transitions
		case BuildStart2:
		state = BuildCharacter2;
		break;
		case BuildCharacter2:
		state = BuildCharacter2;
		break;
		default:
		state = BuildStart2;
		break;
	}
	switch(state){//Actions
		case BuildStart2:
		//initialize bottom
		for (j2 = 1; j2 < 9; j2++) // 9
		{
			for (i2=0; i2<8;i2++)//13
			{
				character2_bottom[j2][i2] = (character2[i2] >> j2);
			}
		}
		//initialize top
		for (j2 = 1; j2 < 9; j2++)
		{
			for (i2=0; i2<8;i2++)
			{
				character2_top[j2][i2] =  (character2[i2] << (8-j2));
			}
		}
		break;
		case BuildCharacter2:
		//build
		if(yLoc2 == 0){
			lcd_setXY(0x80 + xLoc2, 0x40 + yLoc2);
			N5110_image(character2, 8);
			}else{
			lcd_setXY(0x80 + xLoc2 , 0x40 + yLoc2);
			N5110_image(character2_bottom[segLoc2], 8);
			lcd_setXY(0x80 + xLoc2, 0x40 + yLoc2 - 1 );
			N5110_image(character2_top[segLoc2], 8);
		}
		if (segLoc2 == 8)
		{
			segLoc2 = 0;
			yLoc2 --;
		}
		break;
		default:
		break;
	}
	return state;
}

enum ProjStates2 {ProjStart2, ProjWait2, ProjLeft2, ProjRight2};
static unsigned char projectile1[] = {0x01}; static unsigned char projectile2[] = {0x02}; static unsigned char projectile3[] = {0x04}; static unsigned char projectile4[] = {0x08};
static unsigned char projectile5[] = {0x10}; static unsigned char projectile6[] = {0x20}; static unsigned char projectile7[] = {0x40}; static unsigned char projectile8[] = {0x80};
static unsigned char* projectile[] = {projectile8, projectile7, projectile6, projectile5, projectile4, projectile3, projectile2, projectile1};	
static unsigned char erase[] = {0x00};

 char ProjX2;
unsigned char ProjY2;
unsigned char ProjSeg2;
int ProjTick2(int state){
	switch(state){//Transitions
		case ProjStart2:
			state = ProjWait2;
			break;
		case ProjWait2:
			if (!GetBit(PINC, 1) && lastDir2 == 2){
				ProjX2 = xLoc2 + 8;
				ProjSeg2 = segLoc2; //+ 5;
				ProjY2 = yLoc2;
				/*if (ProjSeg > 7)
				{
					ProjSeg -= 7;
					ProjY --;
				}*/
				state = ProjRight2;
			}
			else if (!GetBit(PINC, 1) && lastDir2 == 3){
				ProjX2 = xLoc2 - 1;
				ProjSeg2 = segLoc2; //+ 5;
				ProjY2 = yLoc2;
				/*if (ProjSeg > 7)
				{
					ProjSeg -= 7;
					ProjY --;
				}*/
				state = ProjLeft2;
			}else{
				state = ProjWait2;
			}
			break;
		case ProjRight2:
			if(ProjX2 > 83){
				state = ProjWait2;
			}else{
				state = ProjRight2;
				ProjX2 ++;
			}
			break;
		case ProjLeft2:
			if (ProjX2 == 0)
			{
				state = ProjWait2;
			}
			else {
				state = ProjLeft2;
				ProjX2 --;
			}
			break;
		default:
			state = ProjStart2;
			break;
	}
	switch(state){//Actions
		case ProjStart2:
			ProjX2 = 84;
			ProjY2 = 6;
			ProjSeg2 = 8;
			break;
		case ProjWait2:
			break;
		case ProjRight2:
			lcd_setXY(0x80 + ProjX2 -1  , 0x40 + ProjY2);
			N5110_image(erase, 1);
			lcd_setXY(0x80 + ProjX2 , 0x40 + ProjY2);
			N5110_image(projectile[ProjSeg2], 1);
			break;
		case ProjLeft2:
			if (ProjX2 == 0)
			{
				lcd_setXY(0x80 + 1, 0x40 + ProjY2);
				N5110_image(erase, 1);
			}else {
				lcd_setXY(0x80 + ProjX2  +1, 0x40 + ProjY2);
				N5110_image(erase, 1);
				lcd_setXY(0x80 + ProjX2 , 0x40 + ProjY2);
				N5110_image(projectile[ProjSeg2], 1);
			}
			break;
		default:
			break;
	}
	return state;
}