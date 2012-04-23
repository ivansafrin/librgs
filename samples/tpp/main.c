
#include <rgs.h> 
#include "SDL.h"

#define CONTENT_PATH "data/"

#define ENTITY_PLAYER_START	1
#define ENTITY_DEALER		2
#define ENTITY_CITIZEN		3
#define ENTITY_COP			4
#define ENTITY_HO			5

int smacking = 0;

RGS_Screen *gameScreen;
RGS_Screen *titleScreen;
RGS_Screen *aboutScreen;
RGS_Screen *hiscoreScreen;
RGS_Screen *gameoverScreen;
RGS_Screen *helpScreen;

RGS_SFSSound *jumpSound;
RGS_SFSSound *hitSound;
RGS_SFSSound *selectSound;
RGS_SFSSound *chooseSound;
RGS_SFSSound *illegalSound;
RGS_SFSSound *bustedSound;
RGS_SFSSound *sellSound;

RGS_Tilemap *testLevel;

RGS_Label *cashCounter;
RGS_Label *methCounter;
RGS_Label *cokeCounter;
RGS_Label *moralCounter;
RGS_Label *hoCounter;
RGS_Label *timeCounter;
RGS_Label *wantedLabel;
RGS_Label *fineLabel;
RGS_Label *hiScoreLabel;
RGS_Label *timeUpLabel;
RGS_Label *moralLabel;
RGS_Label *finalScoreLabel;

RGS_Label *hsLabels[10];

RGS_Label *dealerNameLabel;
RGS_Label *dealerCokeLabel;
RGS_Label *dealermethLabel;
RGS_Screen *dealingScreen;
RGS_SurfaceImage *dealArrow;
RGS_SurfaceImage *caneArrow;
int dealMenuOffset = 0;
int titleMenuOffset = 1;

int jumping = 0;
int paused = 0;
int gameover = 0;
int gameoverkeylock = 1;

RGS_Timer moralTimer;
RGS_Timer timeBlinkTimer;
RGS_Timer secondCounter;
RGS_Timer introTimer;

RGS_Timer gameoverTimer;

RGS_Timer smackingTimer;
RGS_Timer fineLabelTimer;

int minutesLeft = 5;
int secondsLeft = 0;
int showingDots = 0;

typedef struct {
	int openCoat;
	int numCoke;
	int cokePrice;
	int nummeth;
	int methPrice;
	int money;
	char name[32];
} Dealer;

int hiScores[10] = { 10000,9000,8000,7000,6000,5000,4000,3000,2000,1000};

typedef struct {
	int walking;
	int direction;
	RGS_Timer moodTimer;
	RGS_Timer highTimer;
	int begging;
	int happy;
	RGS_Entity *entity;
} Citizen;

typedef struct {
	RGS_Timer moodTimer;
	RGS_Timer angryTimer;
	int chasing;
	RGS_Entity *entity;
} Cop;


typedef struct {
	int downout;
	int money;
	RGS_Timer highTimer;
	RGS_Timer moneyTimer;
	RGS_Entity *entity;
} Ho;

typedef struct {
	Dealer *nearestDealer;
	Citizen *nearestCitizen;
	Ho *nearestHo;
	RGS_EntitySprite *pimpSprite;
	int isDealing;
	int money;
	int coke;
	int meth;
	int hos;
	int isWanted;
	int moralint;
} Pimp;

int showingIntro = 1;

Pimp pimp;

#define MAX_CITIZENS  20
#define MAX_COPS  8
#define MAX_HOS  6

#define HOOKER_INCOME 40

Dealer dealers[] = {
{0,5000,800,6000,200,2000,"*LUCKY BORIS*"},
{0,5000,800,6000,200,2000,"*SWEET TOOTH*"}
};
int dealerInit=0;

Citizen citizens[MAX_CITIZENS];
int citizenInit = 0;
Cop cops[MAX_COPS];
int copInit = 0;
Ho hos[MAX_HOS];
int hoInit = 0;

RGS_Timer wantedTimer;
RGS_Timer wantedBlinkTimer;

void Ho_Init(RGS_Entity *entity) {
	RGS_Sprite *spr = (RGS_Sprite*)entity->object;
	RGS_PlaySpriteAnimation(spr, "downout", 0);	
	entity->customData = (void*)&hos[hoInit];
	hos[hoInit].entity = entity;
	hos[hoInit].money = 0;
	hos[hoInit].downout = 1;
	hoInit++;
	
}

void stopWanted() {
			int j;
			for(j =0;j < MAX_COPS;j++) {
				cops[j].chasing = 0;
			}
			pimp.isWanted = 0;
}


void Ho_Update(RGS_Entity *entity) {
	Ho *ho = (Ho*)entity->customData;
	RGS_Sprite *spr = (RGS_Sprite*)entity->object;

	
	if(ho->downout == 1) {
		
	if(RGS_GetEntityDistance(entity, pimp.pimpSprite->entity) < 20) {
		pimp.nearestHo = ho;
	}
		RGS_PlaySpriteAnimation(spr, "downout", 0);		
	} else {
		RGS_UpdateTimer(&ho->highTimer);
		RGS_UpdateTimer(&ho->moneyTimer);	

		if(RGS_TimerHasElapsed(&ho->moneyTimer, 1000) == 1) {
			ho->money += HOOKER_INCOME;
		}

		if(RGS_TimerHasElapsed(&ho->highTimer, 20000) == 1) {
			ho->downout = 1;
			pimp.hos--;
		}
			
		RGS_PlaySpriteAnimation(spr, "idle", 0);		
	}
}


void showHiScore() {
	int k;
	char tmpString[32];
	
	for(k=0;k<10;k++) {
		sprintf(tmpString, "%05d", hiScores[k]);
		RGS_SetLabelText(hsLabels[k], tmpString);
	}
	hiscoreScreen->visible = 1;
}

void Cop_Init(RGS_Entity *entity) {
	RGS_Sprite *spr = (RGS_Sprite*)entity->object;
	RGS_PlaySpriteAnimation(spr, "walk", 0);
	entity->customData = (void*)&cops[copInit];
	RGS_ResetTimer(&cops[copInit].moodTimer);
	RGS_ResetTimer(&cops[copInit].angryTimer);
	cops[copInit].entity = entity;
	copInit++;
}

void Cop_Update(RGS_Entity *entity) {
	RGS_Sprite *spr = (RGS_Sprite*)entity->object;
	Cop *cop = (Cop*)entity->customData;
	RGS_UpdateTimer(&cop->moodTimer);
	
	
	RGS_Tilemap *tilemap = (RGS_Tilemap*)entity->boundTilemap;
	if(cop->chasing == 1) {
	
//		RGS_UpdateTimer(&cop->angryTimer);
//		if(RGS_TimerHasElapsed(&cop->angryTimer, 23000) == 1) {
//			cop->chasing = 0;
//		}
		
		if(RGS_GetEntityDistance(entity, pimp.pimpSprite->entity) < 10) {
			stopWanted();
			RGS_ResetTimer(&fineLabelTimer);
			fineLabel->object.visible = 1;
			RGS_PlaySFS(bustedSound);
			if(pimp.coke > 0 || pimp.meth > 0) {
				if(pimp.money > 200) {
					pimp.coke = 0;
					pimp.meth = 0;
					RGS_SetLabelText(fineLabel, "THE COPS TOOK ALL YOUR DRUGS!!!!!");
				}  else {
					RGS_SetLabelText(fineLabel, "YOURE IN LUCK! THE COPS LET YOU GO");
				}
			} else {
				RGS_SetLabelText(fineLabel, "YOU'RE CLEAN. THE COPS LET YOU GO!");			
			}
		}	
		
		if((RGS_TimerHasElapsed(&cop->moodTimer, 1000) == 1 && rand() % 3  == 1) || entity->x < 2
		   || (entity->xDirection == 1 && entity->tileX > tilemap->mapWidth - 2)) {	
			if(entity->xDirection == 1)
				entity->xDirection = -1;
			else
				entity->xDirection = 1;
			
			if(entity->xDirection == 1)
				spr->flipX = 0;
			else
				spr->flipX = 1;	
		}
		
	} else {
		if((RGS_TimerHasElapsed(&cop->moodTimer, 1000) == 1 && rand() % 15  == 1) || entity->x < 2
		   || (entity->xDirection == 1 && entity->tileX > tilemap->mapWidth - 2)) {	
			if(entity->xDirection == 1)
				entity->xDirection = -1;
			else
				entity->xDirection = 1;
			
			if(entity->xDirection == 1)
				spr->flipX = 0;
			else
				spr->flipX = 1;	
		}
		
	}
	
	if(cop->chasing == 1) {
		entity->xVelocity = 130;
		RGS_PlaySpriteAnimation(spr, "chase", 0);
	} else {
		entity->xVelocity = 90;
		RGS_PlaySpriteAnimation(spr, "walk", 0);
	}
	
}

void Dealer_Init(RGS_Entity *entity) {
	RGS_Sprite *spr = (RGS_Sprite*)entity->object;
	RGS_PlaySpriteAnimation(spr, "idle", 0);
	entity->customData = (void*)&dealers[dealerInit];
	dealerInit++;
}

void Citizen_Update(RGS_Entity *entity) {
	RGS_Sprite *spr = (RGS_Sprite*)entity->object;
	Citizen *citizen = (Citizen*)entity->customData;
	
	RGS_UpdateTimer(&citizen->moodTimer);
	RGS_Tilemap *tilemap = (RGS_Tilemap*)entity->boundTilemap;
	
	if(citizen->happy == 1) {
		RGS_PlaySpriteAnimation(spr, "happy", 0);
		RGS_UpdateTimer(&citizen->highTimer);
		if(RGS_TimerHasElapsed(&citizen->highTimer, 20000) ==1)
			citizen->happy = 0;
		return;
	}
	
	if(RGS_GetEntityDistance(entity, pimp.pimpSprite->entity) < 30) {
		citizen->begging = 1;
		RGS_PlaySpriteAnimation(spr, "idlewant", 0);
		pimp.nearestCitizen = citizen;
		return;
	}
	
	if(pimp.nearestCitizen == citizen)
		pimp.nearestCitizen = NULL;
	
	if(citizen->begging == 1) {
		citizen->begging = 0;
		if(citizen->walking == 1)
			RGS_PlaySpriteAnimation(spr, "walk", 0);
		else
			RGS_PlaySpriteAnimation(spr, "idle", 0);
	}
	
	
	if((RGS_TimerHasElapsed(&citizen->moodTimer, 1000) == 1 && rand() % 10  == 1) || entity->x < 2
	   || (entity->xDirection == 1 && entity->tileX > tilemap->mapWidth - 2)) {
		if(rand() % 2 == 1)
			citizen->direction = 1;
		else
			citizen->direction = -1;
		
		entity->xDirection = citizen->direction;
		if(entity->xDirection == 1)
			spr->flipX = 1;
		else
			spr->flipX = 0;
		
		
		if(citizen->walking == 0) {
			citizen->walking = 1;
			RGS_PlaySpriteAnimation(spr, "walk", 0);
		} else {
			citizen->walking = 0;
			RGS_PlaySpriteAnimation(spr, "idle", 0);
		}
	}
	
	if(citizen->walking == 1)
		entity->xVelocity = 100;
}

void Citizen_Init(RGS_Entity *entity) {
	RGS_Sprite *spr = (RGS_Sprite*)entity->object;
	RGS_PlaySpriteAnimation(spr, "idle", 0);
	entity->customData = (void*)&citizens[citizenInit];
	
	citizens[citizenInit].walking = 0;
	citizens[citizenInit].begging = 0;
	citizens[citizenInit].happy = 0;
	citizens[citizenInit].entity = entity;
	
	if(rand() % 2 == 1)
		citizens[citizenInit].direction = 1;
	else
		citizens[citizenInit].direction = -1;
	
	entity->xDirection = citizens[citizenInit].direction;
	if(entity->xDirection == 1)
		spr->flipX = 1;
	citizenInit++;
	RGS_ResetTimer(&citizens[citizenInit].moodTimer);
}

int copsNearby(RGS_Entity *entity) {
	int i;
	for(i=0;i<MAX_COPS;i++) {
		if(RGS_GetEntityDistance(entity, cops[i].entity) < 80) {
			return 1;
		}
	}
	return 0;
}

void Dealer_Update(RGS_Entity *entity) {
	RGS_Sprite *spr = (RGS_Sprite*)entity->object;
	Dealer *dealer = (Dealer*)entity->customData;
	
	if(copsNearby(entity) == 1 && pimp.isDealing == 1 && pimp.nearestDealer == dealer) {
		pimp.isDealing = 0;
	}
	
	if(RGS_GetEntityDistance(entity, pimp.pimpSprite->entity) < 50) {
		if(dealer->openCoat == 0) {
			RGS_PlaySpriteAnimation(spr, "open", 1);
			pimp.nearestDealer = dealer;		
			dealer->openCoat = 1;
		}
	} else {
		if(dealer->openCoat == 1) {
			if(pimp.nearestDealer == dealer) {
				pimp.nearestDealer = NULL;
			}
			RGS_PlaySpriteAnimation(spr, "close", 1);			
			dealer->openCoat = 0;
		}
	}
}

void dealWithDealer(Dealer *dealer) {
	RGS_SetLabelText(dealerNameLabel, dealer->name);
	char tmpStr[64];
	if(dealer->numCoke > 0) {
		sprintf(tmpStr, "BUY COKE      $%02d/10", dealer->cokePrice);
		dealerCokeLabel->color.r = 255;
		dealerCokeLabel->color.g = 255;
		dealerCokeLabel->color.b = 255;
	} else {
		sprintf(tmpStr, "BUY COKE      ---");
		dealerCokeLabel->color.r = 128;
		dealerCokeLabel->color.g = 128;
		dealerCokeLabel->color.b = 128;	
	}
	RGS_SetLabelText(dealerCokeLabel, tmpStr);
	
	if(dealer->nummeth > 0) {
		sprintf(tmpStr, "BUY METH      $%02d/10", dealer->methPrice);
		dealermethLabel->color.r = 255;
		dealermethLabel->color.g = 255;
		dealermethLabel->color.b = 255;
	} else {
		sprintf(tmpStr, "BUY METH      ---", dealer->methPrice);
		dealermethLabel->color.r = 128;
		dealermethLabel->color.g = 128;
		dealermethLabel->color.b = 128;	
	}	
	RGS_SetLabelText(dealermethLabel, tmpStr);
	dealingScreen->visible = 1;
	dealMenuOffset = 0;
	pimp.isDealing = 1;
	
}

void gameOver() {
	gameover = 1;
	gameoverScreen->visible = 1;
	char tmp[32];
	sprintf(tmp, "%05d", pimp.money);
	RGS_SetLabelText(finalScoreLabel, tmp);
	if(pimp.moralint < 0) {
		moralLabel->object.visible = 1;
		timeUpLabel->object.visible = 0;
	} else {
		moralLabel->object.visible = 0;
		timeUpLabel->object.visible = 1;	
	}

	gameoverkeylock = 1;
	RGS_ResetTimer(&gameoverTimer);

	hiScoreLabel->object.visible = 0;	
	int i;
	int j;
	for(i =0;i<10;i++) {
		if(pimp.money > hiScores[i]) {
			for(j=10; j> i;j--) {
				hiScores[j] = hiScores[j-1];
			}
			hiScores[i] = pimp.money;
			hiScoreLabel->object.visible = 1;	
			return;
		}
	}
}

void doIllegal() {
	int i,j;
	for(i =0;i < MAX_COPS;i++) {
		if(RGS_GetEntityDistance(pimp.pimpSprite->entity, cops[i].entity) < 130 && cops[i].chasing == 0) {
			for(j =0;j < MAX_COPS;j++) {
				cops[j].chasing = 1;
				RGS_ResetTimer(&cops[j].angryTimer);
			}
			RGS_ResetTimer(&wantedTimer);
			pimp.isWanted = 1;
			RGS_PlaySFS(illegalSound);
			return;
		}
	}
	
}

void testSmacko(int testVal) {
	int i;
	for(i =0;i < MAX_COPS;i++) {
		if(testVal > cops[i].entity->x && testVal < cops[i].entity->x+ cops[i].entity->bBox->w) {
			doIllegal();
		}
	}
	for(i =0;i < MAX_HOS;i++) {	
		if(testVal > hos[i].entity->x && testVal < hos[i].entity->x+ hos[i].entity->bBox->w) {
			if(hos[i].money > 0) {
				pimp.money += HOOKER_INCOME;
				hos[i].money -= HOOKER_INCOME;
			}
			pimp.moralint -= 50;
			doIllegal();
		}		
	}
}

void smacko() {
	RGS_ResetTimer(&smackingTimer);
	smacking  =1;
	RGS_PlaySpriteAnimation(pimp.pimpSprite->sprite, "smack", 1);
	RGS_PlaySFS(hitSound);
	if(pimp.pimpSprite->entity->xDirection == 1)
		testSmacko(pimp.pimpSprite->entity->x+26);
	else
		testSmacko(pimp.pimpSprite->entity->x+2);
}

void doMenuChoice() {
				switch(dealMenuOffset) {
					case 1:
						if(pimp.nearestDealer != NULL) {
							if(pimp.money >= pimp.nearestDealer->cokePrice && pimp.nearestDealer->numCoke > 0) {
								pimp.coke += 10;
								pimp.nearestDealer->numCoke -= 10;
								pimp.money -= pimp.nearestDealer->cokePrice;
								pimp.nearestDealer->money += pimp.nearestDealer->cokePrice;
							}
						}
						break;
						case 0:
						if(pimp.nearestDealer != NULL) {
							if(pimp.money >= pimp.nearestDealer->methPrice && pimp.nearestDealer->nummeth > 0) {
								pimp.meth += 10;
								pimp.nearestDealer->nummeth--;
								pimp.money -= pimp.nearestDealer->methPrice;
								pimp.nearestDealer->money += pimp.nearestDealer->methPrice;
							}
						}
						break;
						case 2:
						pimp.isDealing = 0;
						break;
				}

}

void gameScreenKeyPress(int keyCode) {
	switch(keyCode) {
		case RGS_ESCAPE:
			titleScreen->visible = 1;
		break;
		case RGS_BUTTON1:
			if(dealingScreen->visible == 1) {
				doMenuChoice();
			} else {
				if(jumping == 0) {
					RGS_PlaySpriteAnimation(pimp.pimpSprite->sprite, "jump",0);	
					RGS_PlaySFS(jumpSound);
					jumping = 1;
					pimp.pimpSprite->entity->yVelocity = -300;
				}
			}
		break;
		case RGS_BUTTON2:
			if(dealingScreen->visible == 1) {
				doMenuChoice();
			} else {
				smacko();
			}
		break;
		case RGS_START:
			if(dealingScreen->visible == 1) {
				doMenuChoice();
			} else {
				//				if(RGS_isPaused() == 1)
				//					RGS_Pause(0);
				//				else
				//					RGS_Pause(1);
			}
			break;
			case RGS_UP:
			if(dealingScreen->visible == 1) {
				dealMenuOffset--;
				if(dealMenuOffset < 0)
					dealMenuOffset = 2;
			} else {
				if(pimp.nearestDealer != NULL) {
					dealWithDealer(pimp.nearestDealer);
					doIllegal();
				} else if(pimp.nearestHo != NULL && pimp.coke > 0) {
					pimp.nearestHo->downout = 0;
					pimp.coke--;
					pimp.hos++;
					RGS_ResetTimer(&pimp.nearestHo->highTimer);
					RGS_ResetTimer(&pimp.nearestHo->moneyTimer);
					pimp.nearestHo = NULL;
					doIllegal();
				} else {
				
					if(pimp.nearestCitizen != NULL && pimp.meth > 0) {
						pimp.nearestCitizen->happy = 1;
						RGS_ResetTimer(&pimp.nearestCitizen->highTimer);
						pimp.meth--;
						pimp.money += 50;
						RGS_PlaySFS(sellSound);
						pimp.nearestCitizen = NULL;
						doIllegal();
					}
				}
			}
			break;
			case RGS_DOWN:
			if(dealingScreen->visible == 1) {
				dealMenuOffset++;
				if(dealMenuOffset > 2)
					dealMenuOffset = 0;		
			}
			break;
	}	
}

void initPimp() {
	pimp.pimpSprite->entity->y = 300;
	pimp.pimpSprite->entity->x = 0;
	pimp.nearestDealer = NULL;
	pimp.nearestCitizen = NULL;
	pimp.nearestHo = NULL;
	pimp.isDealing = 0;
	pimp.isWanted = 0;
	pimp.coke = 0;
	pimp.hos = 0; 
	pimp.money = 500;
	pimp.meth  = 0;
	pimp.moralint = 999;
}


void startGame() {

	titleScreen->visible = 0;
	gameScreen->visible = 1;
	minutesLeft = 5;
	secondsLeft = 0;
	initPimp();
	gameover = 0;

	hoInit = 0;
	copInit = 0;
	citizenInit = 0;
	
	int i;
	for(i =0;i < MAX_CITIZENS;i++) {
		Citizen_Init(citizens[i].entity);
		citizens[i].entity->x = rand() % testLevel->mapWidth * testLevel->tileSize;
		citizens[i].entity->y = 40*testLevel->tileSize;
	}
	
	for(i =0;i < MAX_COPS;i++) {
		Cop_Init(cops[i].entity);
		cops[i].entity->x = rand() % testLevel->mapWidth * testLevel->tileSize;
		cops[i].entity->y = 40*testLevel->tileSize;
	}

	for(i =0;i < MAX_HOS;i++) {
		Ho_Init(hos[i].entity);
		hos[i].entity->x = rand() % testLevel->mapWidth * testLevel->tileSize;
		hos[i].entity->y = 40*testLevel->tileSize;

	}

}

void titleScreenKeyPress(int keyCode) {
	switch(keyCode) {
		case RGS_START:
			switch(titleMenuOffset) {
				case 0:
					//startGame();
					helpScreen->visible = 1;
					titleScreen->visible = 0;
					RGS_PlaySFS(chooseSound);
					break;
				case 1:
					titleScreen->visible = 0;
					showHiScore();
					break;
				case 2:
					RGS_Exit();
					break;
			}
			break;
		case RGS_UP:
			RGS_PlaySFS(selectSound);
			titleMenuOffset--;
			if(titleMenuOffset < 0)
				titleMenuOffset = 2;
			break;
		case RGS_DOWN:
			RGS_PlaySFS(selectSound);
			titleMenuOffset++;
			if(titleMenuOffset > 2)
				titleMenuOffset = 0;	
			break;
	}
}

void hideIntro() {
	showingIntro = 0;
	RGS_PlayNSF(CONTENT_PATH"pimp.nsf");
	aboutScreen->visible = 0;
	titleScreen->visible = 1;
}

void onKeyPress(int keyCode) {

	if(titleScreen->visible == 1) {
		titleScreenKeyPress(keyCode);
		return;
	}

	if(helpScreen->visible == 1) {
		helpScreen->visible = 0;
		startGame();
		return;
	}
	
	if(gameoverScreen->visible == 1) {
		if(gameoverkeylock == 0) {
			gameoverScreen->visible = 0;
			titleScreen->visible = 1;
		}
		return;
	}

	if(aboutScreen->visible == 1) {
		hideIntro();
		return;
	}

	if(	hiscoreScreen->visible == 1) {
			hiscoreScreen->visible = 0;
			titleScreen->visible = 1;
			return;
	}
	
	
	gameScreenKeyPress(keyCode);
}

void updateHUD() {
	
	char minutesString[16];
	char secondsString[16];
	char tmpString[32];
	
	RGS_UpdateTimer(&timeBlinkTimer);
	RGS_UpdateTimer(&secondCounter);
	
	if(RGS_TimerHasElapsed(&secondCounter, 1000) == 1) {
		secondsLeft--;
		if(secondsLeft < 0) {
			secondsLeft = 59;
			minutesLeft--;
		}
	}
	
	sprintf(secondsString,"%02d", secondsLeft);
	sprintf(minutesString,"%02d", minutesLeft);
	
	
	if(RGS_TimerHasElapsed(&timeBlinkTimer, 500) == 1) {
		if(showingDots == 0) {
			sprintf(tmpString, "TIME: %s:%s", minutesString, secondsString);
			RGS_SetLabelText(timeCounter, tmpString);
			showingDots = 1;
		} else {
			sprintf(tmpString, "TIME: %s %s", minutesString, secondsString);
			RGS_SetLabelText(timeCounter, tmpString);
			showingDots = 0;
		}
	}
	
	sprintf(tmpString,"CASH: $%06d", pimp.money);
	RGS_SetLabelText(cashCounter, tmpString);
	sprintf(tmpString,"COKE: %04d", pimp.coke);
	RGS_SetLabelText(cokeCounter, tmpString);	
	sprintf(tmpString,"METH: %04d", pimp.meth);
	RGS_SetLabelText(methCounter, tmpString);
	sprintf(tmpString,"MORAL INT: %03d", pimp.moralint);
	RGS_SetLabelText(moralCounter, tmpString);	
	sprintf(tmpString,"HOS: %04d", pimp.hos);
	RGS_SetLabelText(hoCounter, tmpString);	
	
	if(pimp.isWanted == 1) {
		RGS_UpdateTimer(&wantedBlinkTimer);
		if(RGS_TimerHasElapsed(&wantedBlinkTimer, 60) == 1) {
			if(wantedLabel->color.r == 0) {
				wantedLabel->color.r  = 252;
				wantedLabel->color.g  = 88;
				wantedLabel->color.b  = 156;
			} else {
				wantedLabel->color.r  = 0;
				wantedLabel->color.g  = 0;
				wantedLabel->color.b  = 196;			
			}
		}
		
		wantedLabel->object.visible = 1;
	} else {
		wantedLabel->object.visible = 0;
	}
}


int main(int argc, char *argv[]) 
{
	RGS_Init(256,240,0,0, RGS_NTSC);
	titleMenuOffset = 0;
	
	srand(SDL_GetTicks());
	
	// sounds
	
	jumpSound = RGS_LoadSFS(CONTENT_PATH"jump.sfs");
	hitSound = RGS_LoadSFS(CONTENT_PATH"hit.sfs");
	selectSound = RGS_LoadSFS(CONTENT_PATH"select.sfs");
	chooseSound = RGS_LoadSFS(CONTENT_PATH"select2.sfs");
	illegalSound = RGS_LoadSFS(CONTENT_PATH"illegal.sfs");
	bustedSound = RGS_LoadSFS(CONTENT_PATH"busted.sfs");
	sellSound = RGS_LoadSFS(CONTENT_PATH"sell.sfs");
	
	gameScreen = RGS_CreateScreen();
	gameScreen->visible = 0;
	titleScreen = RGS_CreateScreen();
	titleScreen->visible = 0;

	hiscoreScreen = RGS_CreateScreen();
	hiscoreScreen->visible = 0;
	
	RGS_CreateShape(hiscoreScreen, RGS_SHAPE_RECT, 0x000000, 0,0,256, 240);
	RGS_CreateLabel(hiscoreScreen, "*** HIGH SCORES ***", RGS_FONT_BIG, 65, 20, 0xffffff);

	int k;
	for(k=0;k<10;k++) {
		hsLabels[k] = RGS_CreateLabel(hiscoreScreen, "000000", RGS_FONT_BIG, 102, 50+(12*k), 0xffffff);
	}

	RGS_CreateLabel(hiscoreScreen, "PRESS ANY KEY", RGS_FONT_BIG, 85, 200, 0xffffff);
	
	RGS_Surface *titleSurface = RGS_LoadSurface(CONTENT_PATH"title.sur");
	RGS_CreateSurfaceImage(titleScreen, titleSurface, 0,0);
	RGS_CreateLabel(titleScreen, "NEW GAME", RGS_FONT_BIG, 160,125, 0xffffff);
	RGS_CreateLabel(titleScreen, "HIGH SCORES", RGS_FONT_BIG, 138,137, 0xffffff);
	RGS_CreateLabel(titleScreen, "EXIT", RGS_FONT_BIG, 190,149, 0xffffff);
	
	RGS_Surface *canearrowSurface = RGS_LoadSurface(CONTENT_PATH"canearrow.sur");
	caneArrow = RGS_CreateSurfaceImage(titleScreen, canearrowSurface, 220, 126);	
	
	aboutScreen = RGS_CreateScreen();
	aboutScreen->visible = 1;
	
	RGS_CreateShape(aboutScreen, RGS_SHAPE_RECT, 0x000000, 0,0,256, 240);
	RGS_CreateLabel(aboutScreen, "COPYRIGHT IVAN SAFRIN 2008", RGS_FONT_BIG, 40, 50, 0xffffff);
	RGS_CreateLabel(aboutScreen, "LICENSED BY TIGSOURCE OF AMERICA", RGS_FONT_BIG, 20, 65, 0xffffff);

	RGS_CreateLabel(aboutScreen, "VGNG IS A REGISTERED TRADEMARK OF", RGS_FONT_BIG, 15, 145, 0xffffff);
	RGS_CreateLabel(aboutScreen, "THE TIGSOURCE CORPORATION", RGS_FONT_BIG, 35, 160, 0xffffff);
	RGS_CreateLabel(aboutScreen, "MUSIC COPYRIGHT 50 CENT", RGS_FONT_BIG, 47, 185, 0xffffff);
	RGS_CreateLabel(aboutScreen, "EVERYTHING ELSE BY IVAN SAFRIN", RGS_FONT_BIG, 28, 200, 0xffffff);
	
	RGS_RegisterEntity(ENTITY_DEALER, gameScreen, CONTENT_PATH"dealer.spr", Dealer_Init, Dealer_Update, NULL, 0, NULL);
	RGS_RegisterEntity(ENTITY_CITIZEN, gameScreen, CONTENT_PATH"citizen2.spr", Citizen_Init, Citizen_Update, NULL, 0, NULL);
	RGS_RegisterEntity(ENTITY_COP, gameScreen, CONTENT_PATH"cop.spr", Cop_Init, Cop_Update, NULL, 0, NULL);
	RGS_RegisterEntity(ENTITY_HO, gameScreen, CONTENT_PATH"ho.spr", Ho_Init, Ho_Update, NULL, 0, NULL);
	
	testLevel = RGS_CreateTilemap(gameScreen, 256, 240, CONTENT_PATH"level1.map", CONTENT_PATH"level1.sur");
	int i;
	for(i =0;i < MAX_CITIZENS;i++) {
		RGS_CreateEntitySpriteByCode(testLevel, ENTITY_CITIZEN, rand() % testLevel->mapWidth, 40);
	}
	
	for(i =0;i < MAX_COPS;i++) {
		RGS_CreateEntitySpriteByCode(testLevel, ENTITY_COP, rand() % testLevel->mapWidth, 40);
	}

	for(i =0;i < MAX_HOS;i++) {
		RGS_CreateEntitySpriteByCode(testLevel, ENTITY_HO, rand() % testLevel->mapWidth, 40);
	}
	
	pimp.pimpSprite = RGS_CreateEntitySprite(gameScreen, testLevel, CONTENT_PATH"pimp2.spr");

	initPimp();

	RGS_PlaySpriteAnimation(pimp.pimpSprite->sprite, "idle",0);
	RGS_TilemapFollowEntity(testLevel, pimp.pimpSprite->entity, 0, -50);
	
	RGS_CreateShape(gameScreen, RGS_SHAPE_RECT, 0x000000, 0,0,256, 27);
	cashCounter = RGS_CreateLabel(gameScreen, "", RGS_FONT_BIG, 2,2, 0xffffff);
	methCounter = RGS_CreateLabel(gameScreen, "", RGS_FONT_BIG, 105,2, 0xffffff);
	cokeCounter = RGS_CreateLabel(gameScreen, "", RGS_FONT_BIG, 180,2, 0xffffff);
	
	moralCounter = RGS_CreateLabel(gameScreen, "", RGS_FONT_BIG, 2,14, 0x808080);
	hoCounter = RGS_CreateLabel(gameScreen, "", RGS_FONT_BIG, 105,14, 0x808080);
	timeCounter = RGS_CreateLabel(gameScreen, "", RGS_FONT_BIG, 180,14, 0x808080);
	
	dealingScreen = RGS_CreateScreen();
	RGS_CreateShape(dealingScreen, RGS_SHAPE_RECT, 0x000000, 15, 40, 160, 75);
	dealingScreen->visible = 0;
	dealerNameLabel = RGS_CreateLabel(dealingScreen, "", RGS_FONT_BIG, 20,45, 0x808080);
	dealerCokeLabel = RGS_CreateLabel(dealingScreen, "", RGS_FONT_BIG, 40,79, 0xffffff);
	dealermethLabel = RGS_CreateLabel(dealingScreen, "", RGS_FONT_BIG, 40,67, 0xffffff);
	RGS_CreateLabel(dealingScreen, "CANCEL", RGS_FONT_BIG, 40,91, 0xffffff);
	RGS_Surface *arrowSurface = RGS_LoadSurface(CONTENT_PATH"arrow.sur");
	dealArrow = RGS_CreateSurfaceImage(dealingScreen, arrowSurface, 30, 68);
	
	RGS_ResetTimer(&timeBlinkTimer);
	RGS_ResetTimer(&secondCounter);
	
	RGS_SetKeyCallback(onKeyPress);

	gameoverScreen = RGS_CreateScreen();
	gameoverScreen->visible = 0;
	
	
	RGS_CreateShape(gameoverScreen, RGS_SHAPE_RECT, 0x000000, 0, 0, 256, 240);
	moralLabel = RGS_CreateLabel(gameoverScreen, "YOU MONSTER! THOSE POOR WOMEN!", RGS_FONT_BIG, 22,45, 0xffffff);
	timeUpLabel = RGS_CreateLabel(gameoverScreen, "TIME'S UP! LETS SEE HOW YOU DID!", RGS_FONT_BIG, 22,45, 0xffffff);
	RGS_CreateLabel(gameoverScreen, "YOUR FINAL SCORE", RGS_FONT_BIG, 70,75, 0xffffff);
	finalScoreLabel = RGS_CreateLabel(gameoverScreen, "99999", RGS_FONT_BIG, 70,95, 0xffffff);
	hiScoreLabel = RGS_CreateLabel(gameoverScreen, "YOU GOT A HIGHSCORE!", RGS_FONT_BIG, 70,115, 0xffffff);
	RGS_CreateLabel(gameoverScreen, "PRESS ANY KEY", RGS_FONT_BIG, 85, 200, 0xffffff);
	
	wantedLabel = RGS_CreateLabel(gameScreen, "WANTED!!!", RGS_FONT_BIG, 100, 40, 0xe40060);
	fineLabel = RGS_CreateLabel(gameScreen, "THE COPS TOOK ALL YOUR DRUGS.", RGS_FONT_BIG, 20, 40, 0xffffff);
	fineLabel->object.visible = 0;
	
	helpScreen = RGS_CreateScreen();
	helpScreen->visible = 0;
	
	RGS_CreateShape(helpScreen, RGS_SHAPE_RECT, 0x000000, 0, 0, 256, 240);
	RGS_CreateLabel(helpScreen, "*** TINY PIMPING 101 *** ", RGS_FONT_BIG, 60,10, 0xffffff);	
	RGS_CreateLabel(helpScreen, "* PRESS UP TO BUY FROM DEALERS", RGS_FONT_BIG, 5,30, 0x808080);	
	RGS_CreateLabel(helpScreen, "* PRESS UP TO SELL METH TO METH HEADS", RGS_FONT_BIG, 5,45, 0xffffff);	
	RGS_CreateLabel(helpScreen, "* PRESS UP TO GIVE COKE TO THE HOS", RGS_FONT_BIG, 5,60, 0x808080);	
	RGS_CreateLabel(helpScreen, "* WITHOUT THE COKE THEY WONT MAKE $$$", RGS_FONT_BIG, 5,75, 0xffffff);	
	RGS_CreateLabel(helpScreen, "* PRESS Z TO JUMP", RGS_FONT_BIG, 5,90, 0x808080);	
	RGS_CreateLabel(helpScreen, "* PRESS X TO SMACK HOS FOR MONEY", RGS_FONT_BIG, 5,105, 0xffffff);	
	RGS_CreateLabel(helpScreen, "* WATCH YOUR MORAL INTEGRITY", RGS_FONT_BIG, 5,120, 0x808080);	
	RGS_CreateLabel(helpScreen, "* IF COPS SEE YOU DO ANYTHING ILLEGAL", RGS_FONT_BIG, 5,135, 0xffffff);	
	RGS_CreateLabel(helpScreen, "    YOU WILL BECOME WANTED", RGS_FONT_BIG, 5,150, 0xffffff);	
	RGS_CreateLabel(helpScreen, "* OTHERWISE THEY WONT TOUCH YOU, PIMP", RGS_FONT_BIG, 5,165, 0x808080);
	RGS_CreateLabel(helpScreen, "* HOW MUCH MONEY CAN YOU MAKE IN 5 MIN?", RGS_FONT_BIG, 5,180, 0xffffff);
	RGS_CreateLabel(helpScreen, "PRESS ANY KEY", RGS_FONT_BIG, 85, 210, 0xffffff);
	
	RGS_ResetTimer(&introTimer);
	RGS_ResetTimer(&moralTimer);
	
	while(RGS_Update() != 0) {
		if((minutesLeft < 0 || pimp.moralint < 0) && gameover == 0) {
			gameOver();
		}

		if(gameoverScreen->visible == 1 && gameoverkeylock == 1) {
			RGS_UpdateTimer(&gameoverTimer);
			if(RGS_TimerHasElapsed(&gameoverTimer, 3000) == 1) {
				gameoverkeylock = 0;
			}
	
		}

		if(pimp.isDealing == 0)
			dealingScreen->visible = 0;
			
		if(smacking == 1){
				RGS_UpdateTimer(&smackingTimer);
				if(RGS_TimerHasElapsed(&smackingTimer, 200) == 1) {
					smacking = 0;
				}
		}	
			
		RGS_UpdateTimer(&moralTimer);
		if(RGS_TimerHasElapsed(&moralTimer, 20) == 1)  {
			if(pimp.moralint < 999) {
				pimp.moralint++;
			}
		}
		
		RGS_UpdateTimer(&wantedTimer);
		if(RGS_TimerHasElapsed(&wantedTimer, 10000) == 1)
			stopWanted();
		
		dealArrow->object.y = 68 + (12*dealMenuOffset);
		caneArrow->object.y = 126 + (12*titleMenuOffset);
		

		if(showingIntro == 1) {
			RGS_UpdateTimer(&introTimer);
			if(RGS_TimerHasElapsed(&introTimer, 5000) == 1)
				hideIntro();
		}
		
		
		if(fineLabel->object.visible == 1) {
			RGS_UpdateTimer(&fineLabelTimer);
			if(RGS_TimerHasElapsed(&fineLabelTimer, 2000) == 1) {
				fineLabel->object.visible = 0;
			}
		}
		
		updateHUD();
		
		if(pimp.isDealing == 1)
			doIllegal();
		
		if(pimp.pimpSprite->entity->isTouchingGround == 1)
			jumping = 0;
		
		if(RGS_KeyDown(RGS_LEFT) == 1 && pimp.isDealing == 0){
			pimp.pimpSprite->entity->xVelocity = 100;
			pimp.pimpSprite->entity->xDirection = -1;
			if(jumping == 0 && smacking == 0)
				RGS_PlaySpriteAnimation(pimp.pimpSprite->sprite, "walk",0);
			pimp.pimpSprite->sprite->flipX = 1;
		} else if(RGS_KeyDown(RGS_RIGHT) == 1 && pimp.isDealing == 0){
			pimp.pimpSprite->entity->xVelocity = 100;
			pimp.pimpSprite->entity->xDirection = 1;	
			pimp.pimpSprite->sprite->flipX = 0;
			if(jumping == 0 && smacking == 0)
				RGS_PlaySpriteAnimation(pimp.pimpSprite->sprite, "walk",0);
		} else {
			if(jumping == 0 && smacking == 0)
				RGS_PlaySpriteAnimation(pimp.pimpSprite->sprite, "idle",0);	
		}
	}
	return 1;
}
