
//includes areas for keyboard control, mouse control, resizing the window
//and draws a spinning rectangle

#include <windows.h>					// Header file for Windows
#include <iostream>
#include "Image_Loading/nvImage.h"		//header file for loading images into textures includes gl and glu header files
#include <cstdlib>						//libraries to allow random values to be generated
#include <stdio.h>
#include <ctime>
#include <mmsystem.h>					//libraries & header files for playing music
#include <mciapi.h>						
#pragma comment(lib, "winmm.lib")

//key variables used throughout program
int	mouse_x = 0, mouse_y = 0;
bool LeftPressed = false;
int screenWidth = 480, screenHeight = 480;
bool keys[256];
float speed = 0;
float speedIncr = 0.0008f;

//enum to track game state
enum gameState { titleScreen, gameScreen, gameOver };
gameState curState;

//texture handles
GLuint texture1 = 0;
GLuint texture2 = 0;
GLuint texture3 = 0;
GLuint texture4 = 0;
GLuint texture5 = 0;
GLuint texture6 = 0;
GLuint texture7 = 0;

//texture scrolling variables
float translationX = 0;
float translationY = 0;

//OBSTACLE ATTRIBUTES

//holds a randomly generated X coordinate value to spawn an obstacle at
float obstacleX = 0;
//holds the Y coordinate used to scroll obstacles
float obstacleY = 0;
//flag set when new obstacle is drawn
bool newObstacle = true;
//flag set at the start of a game when the controls are displayed
bool displayControls = true;
//bool tracking both states of rolling barrel animation
bool curBarrelState = false;
//float holding rolling barrel's x coordinate used to time between frames
float barrelLastChange = 0.0;
//bool holding which direction a rolling barrel will roll in 
//FALSE = LEFT --- TRUE = RIGHT -
bool barrelDirection = false;
//enum holding all different sprites the obstacles can be in
//one instance of obstacleSprite (obstacle) is used to determine the sprite and behaviour of an obstacle
enum obstacleSprite { stinger, car, barrel, rollingBarrel };
obstacleSprite obstacle = stinger;

//SPEED UP ANIMATION

//used to set the amount of time before each speed up
int speedUpTimer = 0;
//used to increase scrolling rates by a given factor at speed ups
float speedFactor = 1.25f;
//flag set to display a speed up animation
bool displaySpeedUp = false;
//X coordinate of speed up animation sprite
float speedUpX = -50;

//ANIMATION TIMERS

//used to time player character animations in game
int gamePlayerAnimationTimer = 0;
//used to time burning car animation in game over screen
int gOverAnimationTimer = 0;
//used to time the police car animation in game
int policeAnimationTimer = 0;

//PLAYER AND SPRITESHEET STATE VARIABLES

//variable tracking player character's X position
float playerPosX = 0;
//enum tracking the current sprite on the spritesheet the player currently is
enum playerSprite { normal, left1, left2, right1, right2, crash1, crash2 };
playerSprite pSprite = normal;

//constants holding x coordinate of barriers either side of the screen
const float rightBarrierX = 48.0;
const float leftBarrierX = -48.0;

//enum holding the two different sprites the police cars can either be in
//one instance of policeSprite (police) is used to determine the sprite of each car
enum policeSprite { state1, state2 };
policeSprite police = state1;

//OPENGL FUNCTION PROTOTYPES
void display();						//called in winmain to draw everything to the screen
void reshape(int width, int height);				//called when the window is resized
void init();						//called in winmain when the program starts.
void processKeys();					//called in winmain to process keyboard input
void update();						//called in winmain to update variables
void displayTitleScreen();			//called in display to display title screen
void displayGameScreen();			//called in display to display game screen
void displayGameOverScreen();		//called in display to display game over screen
void loadTitleScreenTextures();		//called in processKeys to load textures into handles
void loadGameScreenTextures();		//called in processKeys to load textures into handles
void speedDisplay();				//called in update to update motion variables
void buildFont();					//creates a font
void killFont();					//destroys currently active font
void glPrint(const char *fmt, ...);	//prints text to screen

//----IMPORTANT----//
//HDC is defined seperate from win32 variables for use in buildFont()
HDC hDC;
/////////////////////

//holds the number of the first character in score text's display list
GLuint base;

//integer used to record the player's score in each game
int score = 0;

//loads the PNG image specified by args into a texture handle
GLuint loadPNG(char* name)
{
	// Texture loading object
	nv::Image img;

	GLuint myTextureID;

	// Return true on success
	if (img.loadImageFromFile(name))
	{
		glGenTextures(1, &myTextureID);
		glBindTexture(GL_TEXTURE_2D, myTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, img.getInternalFormat(), img.getWidth(), img.getHeight(), 0, img.getFormat(), img.getType(), img.getLevel(0));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);

	}

	else
		MessageBox(NULL, "Failed to load texture", "End of the world", MB_OK | MB_ICONINFORMATION);

	return myTextureID;
}


/*************    START OF OPENGL FUNCTIONS   ****************/
void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	//enable textures throughout
	glEnable(GL_TEXTURE_2D);
	
	//display a different scene depending on gameState
	if (curState == titleScreen)
	{
		displayTitleScreen();
	}
	else if (curState == gameScreen)
	{
		displayGameScreen();
	}
	else if (curState == gameOver)
	{
		displayGameOverScreen();
	}

	glFlush();
}

//displays title screen when called 
void displayTitleScreen()
{
	//draw scrolling sky
	glPushMatrix();

	//applying scrolling effect
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(translationX, 0, 0);
	translationX += 0.00005f;
	glMatrixMode(GL_MODELVIEW);

	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-50, 0);
	glTexCoord2f(0.0, 1.0); glVertex2f(-50, 50);
	glTexCoord2f(1.5, 1.0); glVertex2f(50, 50);
	glTexCoord2f(1.5, 0.0); glVertex2f(50, 0);
	glEnd();

	//sets the texture to repeat if it doesnt fill the whole space
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//reset texture matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glPopMatrix();

	//draw backdrop
	glPushMatrix();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texture3);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-50, -10);
	glTexCoord2f(0.0, 1.0); glVertex2f(-50, 40);
	glTexCoord2f(1.0, 1.0); glVertex2f(50, 40);
	glTexCoord2f(1.0, 0.0); glVertex2f(50, -10);
	glEnd();
	glPopMatrix();

	//title sprite
	glPushMatrix();
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-30, 25);
	glTexCoord2f(0.0, 1.0); glVertex2f(-30, 53);
	glTexCoord2f(1.0, 1.0); glVertex2f(30, 53);
	glTexCoord2f(1.0, 0.0); glVertex2f(30, 25);
	glEnd();
	glPopMatrix();

	//draw scrolling road
	//main road sprite
	glPushMatrix();

	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texture4);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-50, -50);
	glTexCoord2f(0.0, 1.0); glVertex2f(-50, -10);
	glTexCoord2f(1.0, 1.0); glVertex2f(50, -10);
	glTexCoord2f(1.0, 0.0); glVertex2f(50, -50);
	glEnd();
	glPopMatrix();


	//scrolling part of road
	glPushMatrix();

	//applying scrolling effect
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(0, translationY, 0);
	translationY += 0.0005f;
	glMatrixMode(GL_MODELVIEW);

	//removes blank background from texture
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texture5);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-50, -50);
	glTexCoord2f(0.0, 1.0); glVertex2f(-50, -10);
	glTexCoord2f(1.0, 1.0); glVertex2f(50, -10);
	glTexCoord2f(1.0, 0.0); glVertex2f(50, -50);
	glEnd();

	//sets the texture to repeat if it doesnt fill the whole space
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//reset texture matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);

	glPopMatrix();

	//draw car
	glPushMatrix();

	//removes blank background from texture
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTranslatef(0, speed, 0);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texture6);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-20, -40);
	glTexCoord2f(0.0, 1.0); glVertex2f(-20, 0);
	glTexCoord2f(1.0, 1.0); glVertex2f(20, 0);
	glTexCoord2f(1.0, 0.0); glVertex2f(20, -40);
	glEnd();

	glPopMatrix();

	//draw 'press space' text
	glPushMatrix();

	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texture7);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-25, -48);
	glTexCoord2f(0.0, 1.0); glVertex2f(-25, -33);
	glTexCoord2f(1.0, 1.0); glVertex2f(33, -33);
	glTexCoord2f(1.0, 0.0); glVertex2f(33, -48);
	glEnd();

	glPopMatrix();
}

//displays game screen when called
void displayGameScreen()
{
	//every 50 points the game will speed up, increasing the rate at which everything scrolls
	if (curState == gameScreen)
	{
		if (speedUpTimer >= 5)
		{
			//reset timer between speed ups, increase scale factor of speed and set flag to draw speed up animation
			speedUpTimer = 0;
			speedFactor += 0.4f;
			displaySpeedUp = true;
		}
	}

	//draw scrolling background
	glPushMatrix();

		//only scrolls the road if in gameScreen state
		//necessary as this method is called to display game over screen too which has no scrolling effect
		if (curState == gameScreen)
		{
			//applying scrolling effect
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glTranslatef(0, translationY, 0);
			translationY += 0.00023f * speedFactor;
			glMatrixMode(GL_MODELVIEW);
		}

		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(-50, -50);
		glTexCoord2f(0.0, 1.0); glVertex2f(-50, 50);
		glTexCoord2f(1.0, 1.0); glVertex2f(50, 50);
		glTexCoord2f(1.0, 0.0); glVertex2f(50, -50);
		glEnd();

		//sets the texture to repeat if it doesnt fill the whole space
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//reset texture matrix
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

	glPopMatrix();

	//draw obstacles
	glPushMatrix();

		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, texture4);

		if (newObstacle)
		{
			//generate random X coordinate for new obstacle
			obstacleX = -30 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (30 - -30)));

			//randomly determine which obstacle to use
			float randNum = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			
			if (randNum <= 0.25)
			{
				obstacle = stinger;
			}
			else if (randNum <= 0.5)
			{
				obstacle = car;
			}
			else if (randNum <= 0.75)
			{
				obstacle = barrel;
			}
			else
			{
				obstacle = rollingBarrel;
				//set animation states and variables
				curBarrelState = false;
				barrelLastChange = obstacleX;
				if (playerPosX >= obstacleX)
				{
					barrelDirection = true;
				}
				else
				{
					barrelDirection = false;
				}
			}
			newObstacle = false;
		}

		//applying scrolling effect to match background and translating to randomly generated X coordinate
		//effect is applied regardless of the obstacle
		glTranslatef(obstacleX, obstacleY, 0);

		if (curState == gameScreen)
		{
			//implementing different scrolling speeds based on the obstacle type
			if (obstacle == car)
			{
				obstacleY -= 0.017f * speedFactor;
			}
			//rolling barrels will also move in the horizontal plane in the direction of the player when they spawn
			else if (obstacle == rollingBarrel)
			{
				obstacleY -= 0.013f * speedFactor;
				if (barrelDirection)
				{
					obstacleX += 0.003f;
				}
				else
				{
					obstacleX -= 0.003f;
				}
			}
			else
			{
				obstacleY -= 0.013f * speedFactor;
			}
		}

		//if obstacle has passed bottom of screen reset variables ready to spawn new obstacle
		if (obstacleY <= -45)
		{
			//increase score
			score += 10;
			//increase speedTime alongside score
			speedUpTimer++;

			obstacleY = 80;
			newObstacle = true;
		}

		glScalef(0.8f, 0.8f, 0);

		if (obstacle == stinger)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.66); glVertex2f(-20, -10);
			glTexCoord2f(0.0, 1.0); glVertex2f(-20, 20);
			glTexCoord2f(0.5, 1.0); glVertex2f(10, 20);
			glTexCoord2f(0.5, 0.66); glVertex2f(10, -10);
			glEnd();
		}
		else if (obstacle == car)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.5, 0.66); glVertex2f(-20, -10);
			glTexCoord2f(0.5, 1.0); glVertex2f(-20, 20);
			glTexCoord2f(1.0, 1.0); glVertex2f(10, 20);
			glTexCoord2f(1.0, 0.66); glVertex2f(10, -10);
			glEnd();
		}
		else if (obstacle == barrel)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.33); glVertex2f(-20, -10);
			glTexCoord2f(0.0, 0.66); glVertex2f(-20, 20);
			glTexCoord2f(0.5, 0.66); glVertex2f(10, 20);
			glTexCoord2f(0.5, 0.33); glVertex2f(10, -10);
			glEnd();
		}
		//rolling barrels have an animation
		else if (obstacle == rollingBarrel)
		{
			glScalef(0.6f, 0.6f, 0);

			if (curBarrelState)
			{
				glBegin(GL_QUADS);
				glTexCoord2f(0.5, 0.33); glVertex2f(-20, -10);
				glTexCoord2f(0.5, 0.66); glVertex2f(-20, 20);
				glTexCoord2f(1.0, 0.66); glVertex2f(10, 20);
				glTexCoord2f(1.0, 0.33); glVertex2f(10, -10);
				glEnd();
			}
			else
			{
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0); glVertex2f(-20, -10);
				glTexCoord2f(0.0, 0.33); glVertex2f(-20, 20);
				glTexCoord2f(0.5, 0.33); glVertex2f(10, 20);
				glTexCoord2f(0.5, 0.0); glVertex2f(10, -10);
				glEnd();
			}
			//change to next frame if travelled X=10 since last change
			if (abs(obstacleX) - abs(barrelLastChange) >= 5 || abs(obstacleX) - abs(barrelLastChange) <= -5)
			{
				curBarrelState = !curBarrelState;
				barrelLastChange = obstacleX;
			}
		}

	glPopMatrix();

	//draw player car
	glPushMatrix();

		//removes blank background from texture
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//translate player car to the position specified by player button inputs
		glTranslatef(playerPosX, 0, 0);

		glColor3f(1, 1, 1);
		glScalef(0.8f, 0.8f, 0);
		glBindTexture(GL_TEXTURE_2D, texture2);

		//check if player is colliding with either barrier in game screen and trigger game over if so
		if (playerPosX - 17 <= leftBarrierX || playerPosX + 12.5 >= rightBarrierX && curState == gameScreen)
		{
			curState = gameOver;
		}

		//check if player is colliding with an obstacle
		//different obstacles have different sizes
		if (obstacle == car)
		{
			if ((playerPosX + 9 >= obstacleX && obstacleX + 7 >= playerPosX) &&
				(-20 + 14 >= obstacleY && obstacleY + 13 >= -20))
			{
				curState = gameOver;
			}
		}
		else if (obstacle == barrel)
		{
			if ((playerPosX + 12 >= obstacleX && obstacleX + 8 >= playerPosX) &&
				(-20 + 13 >= obstacleY && obstacleY + 15 >= -20))
			{
				curState = gameOver;
			}
		}
		else if (obstacle == stinger)
		{
			if ((playerPosX + 19 >= obstacleX && obstacleX + 14 >= playerPosX) &&
				(-20 + 17 >= obstacleY && obstacleY + 17 >= -20))
			{
				curState = gameOver;
			}
		}
		else if (obstacle == rollingBarrel)
		{
			if ((playerPosX + 9.0 >= obstacleX && obstacleX + 9.0 >= playerPosX) &&
				(-20.0 + 14.0 >= obstacleY && obstacleY + 10.0 >= -20.0))
			{
				curState = gameOver;
			}
		}

		//determining which player sprite to draw
		if (pSprite == normal)
		{
			//reset gPlayerAnimationTimer when not in any animation
			gamePlayerAnimationTimer = 0;

			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.66); glVertex2f(-20, -40);
			glTexCoord2f(0.0, 1.0); glVertex2f(-20, -5);
			glTexCoord2f(0.33, 1.0); glVertex2f(15, -5);
			glTexCoord2f(0.33, 0.66); glVertex2f(15, -40);
			glEnd();
		}
		else if (pSprite == left1)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.33, 0.66); glVertex2f(-20, -40);
			glTexCoord2f(0.33, 1.0); glVertex2f(-20, -5);
			glTexCoord2f(0.66, 1.0); glVertex2f(15, -5);
			glTexCoord2f(0.66, 0.66); glVertex2f(15, -40);
			glEnd();
		}
		else if (pSprite == left2)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.66, 0.66); glVertex2f(-20, -40);
			glTexCoord2f(0.66, 1.0); glVertex2f(-20, -5);
			glTexCoord2f(1.0, 1.0); glVertex2f(15, -5);
			glTexCoord2f(1.0, 0.66); glVertex2f(15, -40);
			glEnd();
		}
		else if (pSprite == right1)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.33); glVertex2f(-20, -40);
			glTexCoord2f(0.0, 0.66); glVertex2f(-20, -5);
			glTexCoord2f(0.33, 0.66); glVertex2f(15, -5);
			glTexCoord2f(0.33, 0.33); glVertex2f(15, -40);
			glEnd();
		}
		else if (pSprite == right2)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.33, 0.33); glVertex2f(-20, -40);
			glTexCoord2f(0.33, 0.66); glVertex2f(-20, -5);
			glTexCoord2f(0.66, 0.66); glVertex2f(15, -5);
			glTexCoord2f(0.66, 0.33); glVertex2f(15, -40);
			glEnd();
		}
		else if (pSprite == crash1)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.66, 0.33); glVertex2f(-20, -40);
			glTexCoord2f(0.66, 0.66); glVertex2f(-20, -5);
			glTexCoord2f(1.0, 0.66); glVertex2f(15, -5);
			glTexCoord2f(1.0, 0.33); glVertex2f(15, -40);
			glEnd();
		}
		else if (pSprite == crash2)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex2f(-20, -40);
			glTexCoord2f(0.0, 0.33); glVertex2f(-20, -5);
			glTexCoord2f(0.33, 0.33); glVertex2f(15, -5);
			glTexCoord2f(0.33, 0.0); glVertex2f(15, -40);
			glEnd();
		}

	glPopMatrix();


	//draw police cars on 3 separate quads
	glPushMatrix();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glScalef(1.3f, 1.3f, 0);

		if (police == state1)
		{
			//car 1
			glPushMatrix();

			glColor3f(1, 1, 1);
			glTranslatef(-20, -33, 0);
			glBindTexture(GL_TEXTURE_2D, texture3);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.5); glVertex2f(-10, -10);
			glTexCoord2f(0.0, 1.0); glVertex2f(-10, 10);
			glTexCoord2f(1.0, 1.0); glVertex2f(10, 10);
			glTexCoord2f(1.0, 0.5); glVertex2f(10, -10);
			glEnd();

			glPopMatrix();

			//car 2
			glPushMatrix();

			glColor3f(1, 1, 1);
			glTranslatef(0, -33, 0);
			glBindTexture(GL_TEXTURE_2D, texture3);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex2f(-10, -10);
			glTexCoord2f(0.0, 0.5); glVertex2f(-10, 10);
			glTexCoord2f(1.0, 0.5); glVertex2f(10, 10);
			glTexCoord2f(1.0, 0.0); glVertex2f(10, -10);
			glEnd();

			glPopMatrix();

			//car 3
			glPushMatrix();

			glColor3f(1, 1, 1);
			glTranslatef(20, -33, 0);
			glBindTexture(GL_TEXTURE_2D, texture3);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.5); glVertex2f(-10, -10);
			glTexCoord2f(0.0, 1.0); glVertex2f(-10, 10);
			glTexCoord2f(1.0, 1.0); glVertex2f(10, 10);
			glTexCoord2f(1.0, 0.5); glVertex2f(10, -10);
			glEnd();

			glPopMatrix();
		}
		else if (police == state2)
		{
			//car 1
			glPushMatrix();

			glColor3f(1, 1, 1);
			glTranslatef(-20, -33, 0);
			glBindTexture(GL_TEXTURE_2D, texture3);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex2f(-10, -10);
			glTexCoord2f(0.0, 0.5); glVertex2f(-10, 10);
			glTexCoord2f(1.0, 0.5); glVertex2f(10, 10);
			glTexCoord2f(1.0, 0.0); glVertex2f(10, -10);
			glEnd();

			glPopMatrix();

			//car 2
			glPushMatrix();

			glColor3f(1, 1, 1);
			glTranslatef(0, -33, 0);
			glBindTexture(GL_TEXTURE_2D, texture3);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.5); glVertex2f(-10, -10);
			glTexCoord2f(0.0, 1.0); glVertex2f(-10, 10);
			glTexCoord2f(1.0, 1.0); glVertex2f(10, 10);
			glTexCoord2f(1.0, 0.5); glVertex2f(10, -10);
			glEnd();

			glPopMatrix();

			//car 3
			glPushMatrix();

			glColor3f(1, 1, 1);
			glTranslatef(20, -33, 0);
			glBindTexture(GL_TEXTURE_2D, texture3);
			glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex2f(-10, -10);
			glTexCoord2f(0.0, 0.5); glVertex2f(-10, 10);
			glTexCoord2f(1.0, 0.5); glVertex2f(10, 10);
			glTexCoord2f(1.0, 0.0); glVertex2f(10, -10);
			glEnd();

			glPopMatrix();
		}

	glPopMatrix();

	//draws speed up animation when flag is set
	if (displaySpeedUp)
	{
		glPushMatrix();

			//translate to x coordinate and increment to make sprite scroll horizontally
			glTranslatef(speedUpX, 0, 0);
			speedUpX += 0.01f;

			glColor3f(1, 1, 1);
			glScalef(1.5f, 1.5f, 0);
			glBindTexture(GL_TEXTURE_2D, texture6);

			//if game over is triggered while sprite is still onscreen, stop drawing it
			if (curState == gameScreen)
			{
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0); glVertex2f(-20, -10);
				glTexCoord2f(0.0, 1.0); glVertex2f(-20, 10);
				glTexCoord2f(1.0, 1.0); glVertex2f(15, 10);
				glTexCoord2f(1.0, 0.0); glVertex2f(15, -10);
				glEnd();
			}

		glPopMatrix();

		//after sprite has scrolled across screen reset X coordinate and reset flag
		if (speedUpX >= 75)
		{
			speedUpX = -50;
			displaySpeedUp = false;
		}
	}

	//draw controls when flag is set
	if (displayControls)
	{
		glPushMatrix();

		//translate to centre of screen
		glTranslatef(5, 0, 0);

		glColor3f(1, 1, 1);
		glScalef(1.5f, 1.5f, 0);
		glBindTexture(GL_TEXTURE_2D, texture7);

		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(-20, -10);
		glTexCoord2f(0.0, 1.0); glVertex2f(-20, 10);
		glTexCoord2f(1.0, 1.0); glVertex2f(15, 10);
		glTexCoord2f(1.0, 0.0); glVertex2f(15, -10);
		glEnd();

		glPopMatrix();

		//after sprite has scrolled across screen reset X coordinate and reset flag
		if (obstacleY <= 50)
		{
			displayControls = false;
		}
	}

	//draw score text to screen in game
	if (curState == gameScreen)
	{
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();

			glColor3f(1.0, 1.0, 0.0);
			glRasterPos2f(-33, 33);
			char str[8];
			sprintf(str, "%d", score);
			glPrint(str);

		glPopMatrix();

		//re-enable textures for next display call
		glEnable(GL_TEXTURE_2D);
	}
}

//adapts displayGameScreen to display game over screen when called
void displayGameOverScreen()
{
	//animating crashed car
	gOverAnimationTimer++;

	if (gOverAnimationTimer <= 500)
	{
		pSprite = crash1;
	}
	else if (gOverAnimationTimer >= 500)
	{
		pSprite = crash2;
	}
	if (gOverAnimationTimer >= 1000)
	{
		gOverAnimationTimer = 0;
	}

	displayGameScreen();

	//drawing game over text
	glPushMatrix();

	glColor3f(1, 1, 1);
	glTranslatef(9, 0, 0);
	glScalef(1.5, 1.5, 0);
	glBindTexture(GL_TEXTURE_2D, texture5);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(-30, -10);
	glTexCoord2f(0.0, 1.0); glVertex2f(-30, 10);
	glTexCoord2f(1.0, 1.0); glVertex2f(20, 10);
	glTexCoord2f(1.0, 0.0); glVertex2f(20, -10);
	glEnd();

	glPopMatrix();

	//draw score text
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();

	glColor3f(1.0, 1.0, 0.0);
	glRasterPos2f(0, 20);
	char str[8];
	sprintf(str, "%d", score);
	glPrint(str);

	glPopMatrix();

	//re-enable textures for next display call
	glEnable(GL_TEXTURE_2D);
}

//loads in textures to their appropriate handles for the title screen
void loadTitleScreenTextures()
{
	texture1 = loadPNG("Sprites/title.png");
	texture2 = loadPNG("Sprites/sky.png");
	texture3 = loadPNG("Sprites/backdropNOSKY1.png");
	texture4 = loadPNG("Sprites/titleScreenRoad.png");
	texture5 = loadPNG("Sprites/repeatingRoad.png");
	texture6 = loadPNG("Sprites/carRearView.png");
	texture7 = loadPNG("Sprites/pressSpace.png");
}

//loads in textures to their appropriate handles for the game screen
//NOTE: any textures needed for game over screen are included here as they use mostly the same range of textures
void loadGameScreenTextures()
{
	texture1 = loadPNG("Sprites/background.png");
	texture2 = loadPNG("Sprites/PlayerSpritesheet.png");
	texture3 = loadPNG("Sprites/PoliceSpritesheet.png");
	texture4 = loadPNG("Sprites/ObstaclesSpritesheet.png");
	texture5 = loadPNG("Sprites/gameOver.png");
	texture6 = loadPNG("Sprites/speedUp.png");
	texture7 = loadPNG("Sprites/controlsText.png");
}

void reshape(int width, int height)		// Resize the OpenGL window
{
	screenWidth = width; screenHeight = height;           // to ensure the mouse coordinates match 
														// we will use these values to set the coordinate system

	glViewport(0, 0, width, height);						// Reset the current viewport

	glMatrixMode(GL_PROJECTION);						// select the projection matrix stack
	glLoadIdentity();									// reset the top of the projection matrix to an identity matrix

	gluOrtho2D(-50, 50, -50.0, 50.0);					// set the coordinate system for the window

	glMatrixMode(GL_MODELVIEW);							// Select the modelview matrix stack
	glLoadIdentity();									// Reset the top of the modelview matrix to an identity matrix
}


//called once at startup
void init()
{
	glClearColor(1.0, 1.0, 0.0, 0.0);						//sets the clear colour to yellow
	srand(static_cast <unsigned> (time(0)));				//sets seed for random number generation

	curState = titleScreen;
	loadTitleScreenTextures();

	//start music
	mciSendString("open \"Music/SearchingForYou.mp3\" type mpegvideo alias mp3", NULL, 0, NULL);
	mciSendString("play mp3 repeat", NULL, 0, NULL);
	
	//build font for text
	buildFont();
}

//handles keyboard input
void processKeys()
{
	//-----TITLE SCREEN-----//
	//start game from title screen
	if (curState == titleScreen && keys[VK_SPACE])
	{
		//load game screen
		curState = gameScreen;
		loadGameScreenTextures();

		//reset variables
		speed = 0;
		translationX = 0;
		translationY = 0;
		playerPosX = 0;
		newObstacle = true;
		displayControls = true;
		//start obstacle for Y = 80 so the controls can be displayed before obstacles start coming
		obstacleY = 100;
	}

	//-----GAME SCREEN-----/
	//set player sprite to normal by default in gameScreen
	if (curState == gameScreen)
	{
		pSprite = normal;

		//move car left in game
		if (keys[VK_LEFT])
		{
			playerPosX -= 0.031f;
			pSprite = left1;

			//increment playerAnimationTimer while key pressed - when greater than 1000 switch sprite
			gamePlayerAnimationTimer++;
			if (gamePlayerAnimationTimer > 1000)
			{
				pSprite = left2;
			}
		}

		//move car right in game
		if (keys[VK_RIGHT])
		{
			playerPosX += 0.031f;
			pSprite = right1;

			//increment playerAnimationTimer while key pressed - when greater than 1000 switch sprite
			gamePlayerAnimationTimer++;
			if (gamePlayerAnimationTimer > 1000)
			{
				pSprite = right2;
			}
		}
	}

	//-----GAME OVER SCREEN-----//
	if (curState == gameOver)
	{
		//moves back to title screen when space pressed
		if (LeftPressed)
		{
			//reset score, speed up timer and speed factor
			score = 0;
			speedUpTimer = 0;
			speedFactor = 1;

			//return to title screen
			curState = titleScreen;
			loadTitleScreenTextures();
		}
	}
}

//called to trigger a game over - displays suitable info to screen and 

//handles motion of sprites
void speedDisplay(void)
{
	speed += speedIncr;
	if (speed > 1 || speed < -1)speedIncr = -speedIncr;
}

//controls police car animation
//uses police variable to determine which sprite to display for each police car
void updatePolice()
{
	policeAnimationTimer++;

	if (policeAnimationTimer <= 500)
	{
		police = state1;
	}
	else if (policeAnimationTimer >= 500)
	{
		police = state2;
	}
	if (policeAnimationTimer >= 1000)
	{
		policeAnimationTimer = 0;
	}
}

//-----TEXT RENDERING-----//
//build the font used to display score
void buildFont()
{
	HFONT font;
	HFONT oldFont;

	//set base to store a list of all 96 characters
	base = glGenLists(96);

	//create font by specifying attributes
	font = CreateFont(-24,			//font size
		0,							//font width (default)
		0,							//rotation on font
		0,							//orientation angle
		FW_BOLD,					//font weight
		FALSE,						//italic
		FALSE,						//underlined
		FALSE,						//strikeout
		ANSI_CHARSET,				//character set
		OUT_TT_PRECIS,				//output precision
		CLIP_DEFAULT_PRECIS,		//clipping precision
		ANTIALIASED_QUALITY,		//antialiasing?
		FF_DONTCARE | DEFAULT_PITCH, //family and pitch
		"Courier New");					//font name

	//select the font to use
	oldFont = (HFONT)SelectObject(hDC, font);
	//builds 96 characters tstarting at 32
	wglUseFontBitmaps(hDC, 32, 96, base);
	//select the font you use and deletes the font ready for the next font
	SelectObject(hDC, oldFont);
	DeleteObject(font);
}

//deletes the font list
//deletes all 96 characters created in createFont
void killFont()
{
	glDeleteLists(base, 96);
}

//prints text to screen with variable arguments
void glPrint(const char *fmt, ...)
{
	//holds string and pointer to list of arguments
	char text[256];
	va_list ap;

	//if no more arguments left do nothing
	if (fmt == NULL)
	{
		return;
	}

	//parses string and converts symbols it may contain to numbers
	va_start(ap, fmt);
		vsprintf_s(text, fmt, ap);
	va_end(ap);

	//push display list bits and sets base to 32 so correct letter can be found
	glPushAttrib(GL_LIST_BIT);
	glListBase(base - 32);

	//draw display list text to screen and pops display list bits
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);
	glPopAttrib();
}

//updates variables
void update()
{
	speedDisplay();
	updatePolice();
}
/**************** END OPENGL FUNCTIONS *************************/

//WIN32 functions
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc
void KillGLWindow();									// releases and destroys the window
bool CreateGLWindow(char* title, int width, int height); //creates the window
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);  // Win32 main function

//win32 global variabless
//HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application


/******************* WIN32 FUNCTIONS ***************************/
int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	bool	done = false;								// Bool Variable To Exit Loop


	AllocConsole();
	FILE *stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);

	// Create Our OpenGL Window
	if (!CreateGLWindow("OpenGL Win32 Example", screenWidth, screenHeight))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while (!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
			{
				done = true;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			if (keys[VK_ESCAPE])
				done = true;

			processKeys();			//process keyboard

			display();					// Draw The Scene
			update();					// update variables
			SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (int)(msg.wParam);						// Exit The Program
}

//WIN32 Processes function - useful for responding to user inputs or other events.
LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
	case WM_CLOSE:								// Did We Receive A Close Message?
	{
		PostQuitMessage(0);						// Send A Quit Message
		return 0;								// Jump Back
	}
	break;

	case WM_SIZE:								// Resize The OpenGL Window
	{
		reshape(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
		return 0;								// Jump Back
	}
	break;

	case WM_LBUTTONDOWN:
	{
		mouse_x = LOWORD(lParam);
		mouse_y = screenHeight - HIWORD(lParam);
		LeftPressed = true;
	}
	break;

	case WM_LBUTTONUP:
	{
		LeftPressed = false;
	}
	break;

	case WM_MOUSEMOVE:
	{
		mouse_x = LOWORD(lParam);
		mouse_y = screenHeight - HIWORD(lParam);
	}
	break;
	case WM_KEYDOWN:							// Is A Key Being Held Down?
	{
		keys[wParam] = true;					// If So, Mark It As TRUE
		return 0;								// Jump Back
	}
	break;
	case WM_KEYUP:								// Has A Key Been Released?
	{
		keys[wParam] = false;					// If So, Mark It As FALSE
		return 0;								// Jump Back
	}
	break;
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void KillGLWindow()								// Properly Kill The Window
{
	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}

	//destroy font on shutdown
	killFont();
	//close currently playing mp3 file on shutdown
	mciSendString("close mp3", NULL, 0, NULL);
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*/

bool CreateGLWindow(char* title, int width, int height)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;											// Return FALSE
	}

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
	dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		24,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		24,											// 24Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	reshape(width, height);					// Set Up Our Perspective GL Screen

	init();

	return true;									// Success
}