
#include <rgs.h>
#include "SDL.h"

//#define CONTENT_PATH "mrdangan.app/Contents/Resources/"
#define CONTENT_PATH "data/"

RGS_Tilemap *gameLevel;
RGS_Screen *gameScreen;
RGS_Screen *titleScreen;

#define MODE_RUNNING 0
#define MODE_STANDING 1
#define MODE_ATTACK 2

RGS_SFSSound *jumpSound;
RGS_SFSSound *hitSound;
RGS_SFSSound *alarmSound;
RGS_SFSSound *crashSound;
RGS_SFSSound *punchSound;

RGS_EntitySprite *killDrop;
int killDropTimingOut = 0;
RGS_Timer killDropTimeout;

RGS_Sprite *incomingSprite;
int incoming = 0;
RGS_Shape *menuSelector;
int menuOffset = 0;

int spectatorMode = 0;
RGS_Timer spectatorTimer;


typedef struct {
	int mode;
	int runDir;
} RobotBrain;

typedef struct {
	RGS_EntitySprite *sprite;
	int hp;
	int jumping;
	int punching;
	RGS_Timer punchingTimer;
	RobotBrain brain;
	RGS_Timer modeChangeTimer;
	RGS_Timer attackingTimer;
	RGS_Timer attackIntervalTimer;	
	RGS_Timer directionChangeTimer;
	int attacking;
	RGS_Timer hpTestTimer;
	int lastHPCheck;
	int dead;
	RGS_SurfaceImage *hpMeter[20];
	int hurt;
	RGS_Timer hurtTimer;
	RGS_Timer hurtBlinkTimer;	
} Robot;

Robot players[8];
Robot *p1;

RGS_Surface *palettes[8];

static int hpOffsetsY[8] = {2,9,2,9,2,9,2,9};
static int hpOffsetsX[8] = {94-20,94-20,173-20,173-20,252-20,252-20,331-20,331-20};


RGS_Timer dropTimer;

void JumpRobot(Robot *robot) {
	if(robot->jumping == 0) {
		RGS_PlaySpriteAnimation(robot->sprite->sprite, "jump",0);	
		robot->jumping = 1;
		robot->sprite->entity->yVelocity = -300;
		RGS_PlaySFS(jumpSound);
	}
}

void SetRobotDirection(Robot *robot, int dir) {

	robot->sprite->entity->xVelocity = 100;
	if(dir == 0)
		robot->sprite->entity->xDirection = 1;			
	else
		robot->sprite->entity->xDirection = -1;
	if(robot->jumping == 0 && robot->punching == 0) {
		RGS_PlaySpriteAnimation(robot->sprite->sprite, "walk",0);
	}
	robot->sprite->sprite->flipX = dir;	
}

Robot *GetRobotInFront(Robot *robot) {
	int i;
	int testVal;
	for(i=0; i < 8; i++) {
		if(&players[i] != robot) {
			if(robot->sprite->entity->xDirection == 1)
				testVal = robot->sprite->entity->x+24;
			else
				testVal = robot->sprite->entity->x+5;
			
			if(testVal > players[i].sprite->entity->x && testVal < players[i].sprite->entity->x+ players[i].sprite->entity->bBox->w && players[i].dead == 0
				&& players[i].hurt == 0) {
				return &players[i];
			}			
			
//			if(RGS_CheckEntityCollision(robot->sprite->entity, players[i].sprite->entity) == 1) {
//				return &players[i];
//			}
		}
	}
	return NULL;
}

void DropStuff() {
	RGS_PlaySFS(alarmSound);
	killDrop->entity->y = 0;
	killDrop->entity->x = 300 + (rand() % 300);
	killDrop->entity->yVelocity = 0;
	killDrop->entity->yVelocityLimit = 2000;
	killDrop->sprite->object.visible = 1;
	killDrop->entity->noGravityOn = 0;
	killDropTimingOut = 0;	
	incoming = 1;
	RGS_PlaySpriteAnimation(killDrop->sprite, "drop", 1);
	incomingSprite->object.visible = 1;
	incomingSprite->object.x= killDrop->sprite->object.x - 20;			
//	robot->sprite->entity->x = 300 + (40*pos);
}

void HurtRobot(Robot *targetRobot, int amt) {
	if(targetRobot->hurt == 1)
		return;
	
	targetRobot->sprite->entity->xVelocity = 100;	
//	if(targetRobot->sprite->entity->isTouchingGround == 1)
//		targetRobot->sprite->entity->yVelocity = -200;	
	
	RGS_PlaySFS(hitSound);
	targetRobot->hp -= amt;

	if(amt > 50) {
		targetRobot->hurt = 1;
		RGS_ResetTimer(&targetRobot->hurtTimer);
		RGS_ResetTimer(&targetRobot->hurtBlinkTimer);	
	}
	
	if(targetRobot->hp <= 0)  {
		targetRobot->hp	= 0;
		if(targetRobot->dead == 0) {
			targetRobot->dead = 1;
			targetRobot->sprite->sprite->object.visible = 0;
		}
	}	
}

void RobotPunch(Robot *robot) {
	RGS_ResetTimer(&robot->punchingTimer);	
	RGS_PlaySpriteAnimation(robot->sprite->sprite, "punch", 1);	
	robot->punching = 1;
	RGS_PlaySFS(punchSound);
	Robot *targetRobot = GetRobotInFront(robot);
	if(targetRobot != NULL) {
		targetRobot->sprite->entity->xDirection = robot->sprite->entity->xDirection;
		HurtRobot(targetRobot, 20);
	}
}

void ResetRobot(Robot *robot, int pos) {
	robot->sprite->entity->x = 300 + (40*pos);
	robot->sprite->entity->y = 230;
	robot->sprite->sprite->object.visible = 1;
	robot->jumping = 1;
	robot->hp = 2000;
	robot->lastHPCheck = robot->hp;
	robot->punching = 0;
	robot->dead = 0;
	robot->brain.mode = 0;
	robot->brain.runDir = rand() % 2;
	robot->attacking = 0;	
	robot->hurt = 0;
	RGS_ResetTimer(&robot->modeChangeTimer);
	RGS_ResetTimer(&robot->directionChangeTimer);
	RGS_PlaySpriteAnimation(robot->sprite->sprite, "jump", 0);	
}

void ResetGame() {
	int i;
	for(i=0; i < 8; i++) {
		ResetRobot(&players[i], i);
	}
	RGS_ResetTimer(&spectatorTimer);	
}

void InitRobot(Robot *robot, int pos) {
	robot->sprite = RGS_CreateEntitySprite(gameScreen, gameLevel, CONTENT_PATH"robot.spr");
	
	if(pos != 0)
		RGS_CopySurfacePalette(robot->sprite->sprite->currentFrame, palettes[pos]);
	
	ResetRobot(robot, pos);
	
	int i;
	for(i = 0; i < 20; i++) {
		robot->hpMeter[i] = RGS_CreateSurfaceImageFromFile(gameScreen, CONTENT_PATH"hp1.sur", hpOffsetsX[pos]+(3*i), hpOffsetsY[pos]);
	}
}

int checkRobotInTilemap(Robot *robot, int xdir, int ydir) {
	if(RGS_GetTileIndex(gameLevel, robot->sprite->entity->tileX+xdir, robot->sprite->entity->tileY+ydir) == TILE_EMPTY) {
		return 1;
	}
	return 0;
}

int checkRobotBarrier(Robot *robot, int xdir, int ydir) {
	if(RGS_GetTileIndex(gameLevel, robot->sprite->entity->tileX+xdir, robot->sprite->entity->tileY+ydir) == TILE_SOLID) {
		return 1;
	}
	return 0;
}


void ThinkRobot(Robot *robot) {	
	if(robot->dead == 1)
		return;
	
//	hpTestTimer
	RGS_UpdateTimer(&robot->hpTestTimer);
	if(RGS_TimerHasElapsed(&robot->hpTestTimer, 1000) == 1) {
		RGS_ResetTimer(&robot->hpTestTimer);
		if(rand() % 2 == 1) {
		if(robot->lastHPCheck - robot->hp > 30) {
			robot->brain.mode = MODE_RUNNING;
			robot->attacking = 0;
			RGS_ResetTimer(&robot->modeChangeTimer);			
//			JumpRobot(robot);			
		}
		}
		robot->lastHPCheck = robot->hp;
	}
	
	RGS_UpdateTimer(&robot->modeChangeTimer);
	if(RGS_TimerHasElapsed(&robot->modeChangeTimer, 3000) == 1) {
		RGS_ResetTimer(&robot->modeChangeTimer);
		if(rand() % 5 == 1)
			robot->brain.mode = MODE_STANDING;
		else if(rand() % 5 == 2)
			robot->brain.mode = MODE_RUNNING;
		else
			robot->brain.mode = MODE_ATTACK;
	}
	
	if(robot->jumping == 0) {
	if(robot->sprite->entity->xDirection == -1) {
		if(checkRobotInTilemap(robot, -1, 1)) {
			if(checkRobotBarrier(robot, -2, 4) || checkRobotBarrier(robot, -3, 0)) {
				JumpRobot(robot);
			} else {
				robot->brain.runDir = 0;
			}
		} else {
			if(checkRobotBarrier(robot, -1, -1)) {
				JumpRobot(robot);
			}
		}
	} else {
		if(checkRobotInTilemap(robot, 2, 1)) {
			if(checkRobotBarrier(robot, 3, 4) || checkRobotBarrier(robot, 3, 0)) {
				JumpRobot(robot);
			} else {				
				robot->brain.runDir = 1;
			}
		} else {
			if(checkRobotBarrier(robot, 4, -1)) {
				JumpRobot(robot);
			}			
		}
	}	
	}
	
	if(robot->brain.mode == MODE_ATTACK) {
		RGS_UpdateTimer(&robot->attackingTimer);
		RGS_UpdateTimer(&robot->attackIntervalTimer);		
		
		if(robot->attacking) {
			Robot *targetRobot = GetRobotInFront(robot);
			if(targetRobot == NULL) {
				robot->attacking = 0;
			} else {
			
			if(RGS_TimerHasElapsed(&robot->attackingTimer, 2000) == 1) {
				robot->attacking = 0;
			} else {
				if(RGS_TimerHasElapsed(&robot->attackIntervalTimer, 200) == 1) {
					RobotPunch(robot);
					RGS_ResetTimer(&robot->attackIntervalTimer);						
				}
			}
			}
		} else {
			Robot *targetRobot = GetRobotInFront(robot);
			if(targetRobot != NULL) {
				RGS_ResetTimer(&robot->attackingTimer);
				RGS_ResetTimer(&robot->attackIntervalTimer);			
				robot->attacking = 1;
			}
		}
	}
	
	switch(robot->brain.mode) {
		default:
		case MODE_RUNNING:
		case MODE_ATTACK:		
			RGS_UpdateTimer(&robot->directionChangeTimer);
			if(RGS_TimerHasElapsed(&robot->directionChangeTimer, 3000) == 1) {
				if(rand() % 3 == 1) {
					RGS_ResetTimer(&robot->directionChangeTimer);
					if(robot->brain.runDir == 0)
						robot->brain.runDir = 1;
					else
						robot->brain.runDir = 0;
				}
			}
			
			if(incoming == 1) {
				if(robot->sprite->entity->x > killDrop->entity->x - 50 && 
				   robot->sprite->entity->x < killDrop->entity->x + 50 && robot->attacking == 1) {
					robot->brain.mode = MODE_RUNNING;
					RGS_ResetTimer(&robot->modeChangeTimer);
				}
				
				if(robot->brain.runDir == 0) {
					if(killDrop->entity->x > robot->sprite->entity->x && 
					   killDrop->entity->x < robot->sprite->entity->x + 50) {
						robot->brain.runDir = 1;
					}
				} else {
					if(killDrop->entity->x > robot->sprite->entity->x - 50&& 
					   killDrop->entity->x < robot->sprite->entity->x) {
						robot->brain.runDir = 0;
					}					
				}
				
			}

			//if(robot->sprite->entity->isTouchingGround == 1)
			if(robot->brain.mode == MODE_ATTACK) {
				if(robot->attacking == 0)
					SetRobotDirection(robot, robot->brain.runDir);
			} else {
					SetRobotDirection(robot, robot->brain.runDir);				
			}
		break;
		case MODE_STANDING:
				RGS_PlaySpriteAnimation(robot->sprite->sprite, "idle",0);
				if(incoming == 1) {
					if(robot->sprite->entity->x > killDrop->entity->x - 50 && 
					   robot->sprite->entity->x < killDrop->entity->x + 50) {
						robot->brain.mode = MODE_RUNNING;
					}
				}
					
		break;
	}

	
}

void UpdateRobot(Robot *robot) {
	if(robot->punching == 1){
		RGS_UpdateTimer(&robot->punchingTimer);
		if(RGS_TimerHasElapsed(&robot->punchingTimer, 200) == 1) {
			robot->punching = 0;
		}
	}
	
	if(robot->hurt == 1) {
		RGS_UpdateTimer(&robot->hurtTimer);
		RGS_UpdateTimer(&robot->hurtBlinkTimer);

		if(RGS_TimerHasElapsed(&robot->hurtBlinkTimer, 90) == 1) {
			if(robot->sprite->sprite->object.visible == 0)
				robot->sprite->sprite->object.visible = 1;
			else
				robot->sprite->sprite->object.visible = 0;
		}				
		
		if(RGS_TimerHasElapsed(&robot->hurtTimer, 1000) == 1) {
			robot->hurt = 0;
			if(robot->dead == 0)
				robot->sprite->sprite->object.visible = 1;
			else
				robot->sprite->sprite->object.visible = 0;
		}		
	}
	
	int i;
	for(i=0; i < 20; i++) {
		if(robot->hp/100 > i) {
			robot->hpMeter[i]->object.visible = 1;
		} else {
			robot->hpMeter[i]->object.visible = 0;			
		}
	}
	
	if(robot->sprite->entity->isTouchingGround == 1) {
		robot->jumping = 0;	
	}
	
	if(robot->sprite->entity->y > 1000) {
		robot->hp = 0;
		robot->dead = 1;
		robot->sprite->sprite->object.visible = 0;
	}
}

void QuitToMenu() {
	titleScreen->visible = 1;
	gameScreen->visible = 0;
	RGS_PlayNSF(CONTENT_PATH"menu.nsf");
}

void GameStart() {
	titleScreen->visible = 0;
	gameScreen->visible = 1;
	ResetGame();
	RGS_PlayNSF(CONTENT_PATH"level.nsf");	
}

void titleScreenKey(int key) {
	switch(key) {
		case RGS_UP:
			menuOffset--;
			break;
		case RGS_DOWN:
			menuOffset++;			
			break;
		case RGS_START:
			switch(menuOffset) {
				case 0:
					GameStart();
				break;		
				case 1:
				break;					
				case 2:
				break;					
				case 3:
					RGS_Exit();
				break;					

			}
			break;
	}
	
	if(menuOffset < 0)
		menuOffset = 3;
	if(menuOffset > 3)
		menuOffset = 0;
	
	menuSelector->object.y = 175 + (14*menuOffset);
	
}

void gameScreenKey(int key) {
	switch(key) {
		case RGS_LEFT:
			break;
		case RGS_RIGHT:
			break;
		case RGS_BUTTON1:
			JumpRobot(p1);
			break;
		case RGS_BUTTON2:
			RobotPunch(p1);
			break;
		case RGS_ESCAPE:
			QuitToMenu();
		break;
	}	
}


void keyCallback(int key) {
	if(titleScreen->visible == 1)
		titleScreenKey(key);
	if(gameScreen->visible == 1)
		gameScreenKey(key);
}

void GameLoop() {
	int i;
	for(i=0; i < 8; i++) {
		UpdateRobot(&players[i]);
		if(i == 0) {
			if(spectatorMode == 1)
				ThinkRobot(&players[i]);							
		} else {
			ThinkRobot(&players[i]);			
		}
	}
	
	// update killdrop
	incomingSprite->object.x= killDrop->sprite->object.x - 20;		
	if(killDropTimingOut == 1) {
		RGS_UpdateTimer(&killDropTimeout);
		if(RGS_TimerHasElapsed(&killDropTimeout, 2000) == 1) {
			killDropTimingOut = 0;
			killDrop->entity->y = -1000;
			killDrop->entity->noGravityOn = 1;
			killDrop->entity->isTouchingGround = 0;
		}
	}
	
	if(killDrop->entity->isTouchingGround == 1 && killDropTimingOut == 0) {
		incoming = 0;
		for(i=0; i < 8; i++){ 
			if(RGS_CheckEntityCollision(players[i].sprite->entity, killDrop->entity) == 1) {
				HurtRobot(&players[i], 400);
			}
		}
		RGS_PlaySpriteAnimation(killDrop->sprite, "rest", 1);
		killDropTimingOut = 1;
		RGS_PlaySFS(crashSound);
		incomingSprite->object.visible = 0;	
		RGS_ResetTimer(&killDropTimeout);
	}
	
	if(spectatorMode == 1) {
		RGS_UpdateTimer(&spectatorTimer);
		if(RGS_TimerHasElapsed(&spectatorTimer, 5000) == 1) {
			RGS_ResetTimer(&spectatorTimer);
			int randval = rand() % 8;
			RGS_Entity *rent = players[randval].sprite->entity;			
			if(players[randval].dead == 0)
				RGS_TilemapFollowEntity(gameLevel, rent, 0, 0);
		}
	}
	
	RGS_UpdateTimer(&dropTimer);
	if(RGS_TimerHasElapsed(&dropTimer, 3000) == 1) {
		RGS_ResetTimer(&dropTimer);
		///			if(rand() % 2 == 1) {
		DropStuff();
		//		}
	}
	
	if(p1->dead == 0 && spectatorMode == 0) {
		if(RGS_KeyDown(RGS_LEFT) == 1){
			SetRobotDirection(p1, 1);
		} else if(RGS_KeyDown(RGS_RIGHT) == 1){
			SetRobotDirection(p1, 0);			
		} else {
			if(p1->punching == 0 && p1->jumping == 0)
				RGS_PlaySpriteAnimation(p1->sprite->sprite, "idle",0);
		}	
	}	
}

int main(int argc, char *argv[])
{
	srand(SDL_GetTicks());
	
	RGS_Init(420,240,840,480, RGS_NORMAL);
	RGS_SetKeyCallback(keyCallback);
	
	jumpSound = RGS_LoadSFS(CONTENT_PATH"jump.sfs");
	hitSound = RGS_LoadSFS(CONTENT_PATH"hit.sfs");
	alarmSound = RGS_LoadSFS(CONTENT_PATH"alarm.sfs");
	crashSound = RGS_LoadSFS(CONTENT_PATH"crash.sfs");
	punchSound = RGS_LoadSFS(CONTENT_PATH"punch.sfs");
	
	palettes[1] = RGS_LoadSurface(CONTENT_PATH"pal_purple.sur");
	palettes[2] = RGS_LoadSurface(CONTENT_PATH"pal_green.sur");
	palettes[3] = RGS_LoadSurface(CONTENT_PATH"pal_red.sur");
	palettes[4] = RGS_LoadSurface(CONTENT_PATH"pal_blue.sur");
	palettes[5] = RGS_LoadSurface(CONTENT_PATH"pal_gray.sur");
	palettes[6] = RGS_LoadSurface(CONTENT_PATH"pal_cyan.sur");
	palettes[7] = RGS_LoadSurface(CONTENT_PATH"pal_brown.sur");	
	
	titleScreen = RGS_CreateScreen();
	
	RGS_CreateSurfaceImageFromFile(titleScreen, CONTENT_PATH"menu.sur", 0,0);
	
	menuSelector = RGS_CreateShape(titleScreen, RGS_SHAPE_RECT, 0xca521d, 149, 175, 123, 14);
	RGS_CreateSurfaceImageFromFile(titleScreen, CONTENT_PATH"menu1.sur", 191,177);

	
	RGS_Sprite *titleanim = RGS_CreateSprite(titleScreen, CONTENT_PATH"title.spr");
	RGS_PlaySpriteAnimation(titleanim, "titleani", 0);
	titleanim->object.x = 118;
	titleanim->object.y = 18;	
	
	gameScreen = RGS_CreateScreen();
	gameScreen->visible = 0;

	
	RGS_CreateSurfaceImageFromFile(gameScreen,CONTENT_PATH"storagebg.sur", 0,0);
	gameLevel = RGS_CreateTilemap(gameScreen, 420, 240, CONTENT_PATH"storage.map", CONTENT_PATH"storage.sur");

	RGS_CreateSurfaceImageFromFile(gameScreen, CONTENT_PATH"hud1.sur", 80-20,2);
	
	killDrop = RGS_CreateEntitySprite(gameScreen, gameLevel, CONTENT_PATH"crate.spr");
	
	incomingSprite = RGS_CreateSprite(gameScreen, CONTENT_PATH"incoming.spr");
	incomingSprite->object.y = 20;
	incomingSprite->object.visible = 0;
	
	RGS_ResetTimer(&dropTimer);
	
	int i;
	for(i=0; i < 8; i++) {
		InitRobot(&players[i], i);
	}
	
	
	RGS_TilemapFollowEntity(gameLevel, players[0].sprite->entity, 0, 0);
	p1 = &players[0];
	
	QuitToMenu();
	
	while(RGS_Update() != 0) {
		if(gameScreen->visible == 1)
			GameLoop();
	}

	return 1;
}

