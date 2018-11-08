/*
######### MORPH ##########
#(C) 2018 Joonas Lindberg#
#   All Rights Reserved  #
##########################
*/

// In order for this game to compile, you must have the SDL libraries which can be downloaded for free
// in the official SDL website, https://www.libsdl.org/

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

// Below are various definition constants.
const int delayforscreenTransition = 12;
// Player directions.
const char dirUP = 0;
const char dirDOWN = 1;
const char dirLEFT = 2;
const char dirRIGHT = 3;
const int scrolltextY = 390; // Y position of title screen scroll text.
// Meanings of game states.
const int stateGAME = 0;
const int stateMOVING = 1;
const int stateRETRY = 2;
const int stateLEVELCOMPLETE = 3;
const int stateENDSCREEN = 4;
const int stateTITLE = 5;
// Tile related definitions.
const int tiledefSpace = 0;
const int tiledefball1 = 1;
const int tiledefball2 = 2;
const int tiledefball3 = 3;
const int tiledefball4 = 4;
const int tiledefball5 = 5;
const int tiledefball6 = 6;
const int tiledefball7 = 7;
const int tiledefball8 = 8;
const int tiledefWall = 9;
const int tileColumns = 16;
const int tileLines = 11;
const int tileWidth = 50;
const int tileHeight = 50;
const int SCREEN_WIDTH = 800; // Game screen width
const int SCREEN_HEIGHT = 600; // Game screen height
// Look up tables for quicker retrieval of X,Y coordinates.
const int tilesetXoffsets[16] = { 0, 50, 100, 150, 0, 50, 100, 150, 0, 50, 100, 150, 0, 50, 100, 150 }; // Tile X position lookup table
const int tilesetYoffsets[16] = { 0, 0, 0, 0, 50, 50, 50, 50, 100, 100, 100, 100, 150, 150, 150, 150 }; // Tile Y position lookup table

int playerX, playerY, tiletype, gameState, movingballX, movingballY, screenTransition1pos, screenTransition2pos, screenTransitionDelay,
screenTransitionPhase, level, tryagainY, tryagainH, delay, titlescrollOffset, scrollingtextPos;

// X,Y coordinates of animated gfx objects.
// A negative X coordinate means: this gfx object has reached the end of its animation frames.
int animObj[20];
// In this game, I have what I call "AnimObjects" - my own call name for "animated graphics objects".
// These here are pointers which indicate which frame is shown for each AnimObject.
int animObjindex[10];
int playerDir = dirUP;
int movingballColour = 0;
int movementUnits = 0;
int xmovementDelta = 0;
int ymovementDelta = 0;
int playerXpos = 0;
int playerYpos = 0;
int objSpeed = 5;

// All the levels of the game.
char levelData[2670];

// The current level as tiles.
int gameArea[176];

// The scroll text is stored in this array. The 9th element is the letter, number or punctuation character
// that will appear next from the right side of the screen.
unsigned char scrolltextArray[9] = {' ',' ',' ',' ',' ',' ',' ',' ',' '};

const std::string titleText = "MORPH......  WRITTEN IN 2018 BY  JOONAS LINDBERG       GET RID OF ALL THE BALLS BY PUSHING THEM AGAINST OTHER BALLS OF THE SAME COLOUR....... IN-GAME KEYS:  ARROW KEYS TO MOVE..... PRESS 'R' TO RESTART THE LEVEL......  HAVE FUN AND VISIT MY WEBSITE:   JOONASLINDBERG.FI     THIS TEXT WILL NOW LOOP......";

SDL_Texture* usableTexture = NULL;
SDL_Surface* image1 = NULL;
SDL_Surface* image2 = NULL;
SDL_Surface* image3 = NULL;
SDL_Surface* unchangedBg = NULL;
SDL_Surface* tilegfx = NULL;
SDL_Surface* gameAnimations = NULL;
SDL_Surface* blankness = NULL;
SDL_Surface* endscreen = NULL;
SDL_Surface* titlescreen = NULL;
SDL_Surface* gameFont = NULL;
Mix_Chunk *morphsnd1 = NULL;
Mix_Chunk *morphsnd2 = NULL;
Mix_Chunk *morphsnd3 = NULL;

// Texture wrapper class
class LTexture
{
public:
	// Initializes variables
	LTexture();
	// Deallocates memory
	~LTexture();
	// Deallocates texture
	void free();
private:
	// The actual hardware texture
	SDL_Texture* mTexture;
	// Image dimensions
	int mWidth;
	int mHeight;
};

bool init();
void close();
SDL_Window* gWindow = NULL;  // The window we'll be rendering to
SDL_Renderer* gRenderer = NULL; // The window renderer
LTexture gImageTexture; // Scene textures

LTexture::LTexture()
{
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}
LTexture::~LTexture()
{
	free();
}

// Free texture if it exists.
void LTexture::free()
{
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

bool init()
{
	bool success = true; // Must stay "true" during init so we can start executing the game.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}
		gWindow = SDL_CreateWindow("Morph", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			// I use VSync, so that the refresh rate is beautifully smooth.
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			}
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
			{
				printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
				success = false;
			}
		}
	}
	return success;
}

void close()
{
	gImageTexture.free();
	Mix_FreeChunk(morphsnd1);
	morphsnd1 = NULL;
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;
	Mix_Quit();
	SDL_Quit();
}

bool loadMedia()
{
	bool success = true;
	// Player sprite gfx.
	image1 = SDL_LoadBMP("data/morphgfx2.bmp");
	if (image1 == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Screen buffer.
	image2 = SDL_LoadBMP("data/morphgfx1.bmp");
	if (image2 == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Another screen buffer.
	image3 = SDL_LoadBMP("data/morphgfx1.bmp");
	if (image3 == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Unchanging screen background gfx.
	unchangedBg = SDL_LoadBMP("data/morphgfx1.bmp");
	if (unchangedBg == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Tile gfx file.
	tilegfx = SDL_LoadBMP("data/morphgfx3.bmp");
	if (tilegfx == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Gfx for the AnimObjects.
	gameAnimations = SDL_LoadBMP("data/morphgfx4.bmp");
	if (gameAnimations == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// End screen gfx.
	endscreen = SDL_LoadBMP("data/morphgfx5.bmp");
	if (endscreen == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Title screen gfx.
	titlescreen = SDL_LoadBMP("data/morphgfx6.bmp");
	if (titlescreen == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Gfx for the "Try Again" screen.
	blankness = SDL_LoadBMP("data/morphgfx7.bmp");
	if (blankness == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Gfx for the font used in the title screen scroll text.
	gameFont = SDL_LoadBMP("data/morphgfx8.bmp");
	if (gameFont == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	// Sound effect.
	morphsnd1 = Mix_LoadWAV("data/morphsnd1.wav");
	if (morphsnd1 == NULL)
	{
		printf("SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	// Sound effect.
	morphsnd2 = Mix_LoadWAV("data/morphsnd2.wav");
	if (morphsnd2 == NULL)
	{
		printf("SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	// Sound effect.
	morphsnd3 = Mix_LoadWAV("data/morphsnd3.wav");
	if (morphsnd3 == NULL)
	{
		printf("SDL_mixer Error: %s\n", Mix_GetError());
		success = false;
	}
	return success;
}

// We reset title screen settings every time we go back to the title screen.
void initTitle()
{
	scrolltextArray[0] = ' ';
	scrolltextArray[1] = ' ';
	scrolltextArray[2] = ' ';
	scrolltextArray[3] = ' ';
	scrolltextArray[4] = ' ';
	scrolltextArray[5] = ' ';
	scrolltextArray[6] = ' ';
	scrolltextArray[7] = ' ';
	scrolltextArray[8] = ' ';
	titlescrollOffset = 0;
	scrollingtextPos = 0;
}

// Initialise the screen transition effect.
void screentransitionInit()
{
	screenTransition1pos = 0;
	screenTransition2pos = 1320000;
	screenTransitionDelay = delayforscreenTransition;
	screenTransitionPhase = 5;
}

// Do the screen transition. Call screentransitionInit() first.
void screenTransition()
{
	unsigned char* pixelArray = (unsigned char*)image3->pixels;
	int pos = screenTransition1pos;
	int pos2 = screenTransition2pos;
	for(int offset = 0; offset < 120000; offset++)
	{
		if (pixelArray[pos] > 233)
		{
			pixelArray[pos] = 255;
		}
		else
		{
			pixelArray[pos] += 22;
		}
		if (pixelArray[pos2] > 233)
		{
			pixelArray[pos2] = 255;
		}
		else
		{
			pixelArray[pos2] += 22;
		}
		pos++;
		pos2++;
	}
	screenTransitionDelay--;
	if (screenTransitionDelay == 0)
	{
		if (screenTransitionPhase > 0)
		{
			screenTransitionDelay = delayforscreenTransition;
			screenTransition1pos += 120000;
			screenTransition2pos -= 120000;
			screenTransitionPhase--;
			Mix_PlayChannel(-1, morphsnd3, 0);
		}
	}
	image3->pixels = pixelArray;
}

// Sets the absolute screen X,Y position of the player according to the tile X,Y position
// (given in playerX, playerY).
void playerexactScreenlocation()
{
	playerXpos = 0;
	playerYpos = 0;
	int px = 0;
	int py = 0;
	while (px < playerX)
	{
		playerXpos += tileWidth;
		px++;
	}
	while (py < playerY)
	{
		playerYpos += tileHeight;
		py++;
	}
}

// Change the given value positive if it's negative.
int noNeg(int givenValue)
{
	int result = givenValue;
	if (givenValue < 0) result = 256 + givenValue;
	return result;
}

// Zero all the AnimObjects.
void noAnim()
{
	int pos = 0;
	while (pos < 20)
	{
		animObj[pos] = -1;
		pos++;
	}
}

// Load the current level.
void loadLevel()
{
	SDL_Rect blitDest;
	SDL_Rect areatoBlit;
	int arewethere, gridcol, gridline;
	bool emptytile;
	int tilecol = 0;
	int tileline = 0;
	int leveldataOffset = 0;
	int gameareaOffset = 0;
	int findinglevel = level - 1;
	bool lowernybble = false;
	noAnim();
	SDL_BlitSurface(unchangedBg, NULL, image3, NULL);
	while (findinglevel > 0)
	{
		leveldataOffset += 89;
		findinglevel--;
	}
	areatoBlit.w = tileWidth;
	areatoBlit.h = tileHeight;
	blitDest.x = 0;
	blitDest.y = 0;
	// Determine player X,Y coordinates from the starting tile in the level data.
	playerX = 0;
	playerY = 0;
	while (tilecol < noNeg(levelData[leveldataOffset]))
	{
		playerX++;
		if (playerX >= tileColumns)
		{
			playerX = 0;
			playerY++;
		}
		tilecol++;
	}
	playerexactScreenlocation();
	tilecol = 0;
	leveldataOffset++;
	while (tileline < tileLines)
	{
		emptytile = false;
		tiletype = noNeg(levelData[leveldataOffset]);
		if (lowernybble == false)
		{
			tiletype = tiletype & 0xF0;
			tiletype = tiletype >> 4;
		}
		tiletype = tiletype & 0xF;
		if (tiletype < tiledefWall) emptytile = true;
		if (lowernybble == false)
		{
			lowernybble = true;
		}
		else
		{
			lowernybble = false;
			leveldataOffset++;
		}
		gameArea[gameareaOffset] = tiletype;
		areatoBlit.x = 0;
		areatoBlit.y = 0;
		// Tile gfx file = grid of 4 x 4 tiles.
		arewethere = 0;
		gridcol = 0;
		gridline = 0;
		while (arewethere < tiletype)
		{
			areatoBlit.x += tileWidth;
			gridcol++;
			if (gridcol >= 4)
			{
				areatoBlit.x = 0;
				areatoBlit.y += tileHeight;
				gridcol = 0;
				gridline++;
			}
			arewethere++;
		}
		if(!emptytile) SDL_BlitSurface(tilegfx, &areatoBlit, image3, &blitDest);
		blitDest.x += tileWidth;
		tilecol++;
		if (tilecol >= tileColumns)
		{
			blitDest.x = blitDest.x - (tileWidth * tileColumns);
			blitDest.y += tileHeight;
			tilecol = 0;
			tileline++;
		}
		gameareaOffset++;
	}
	gameState = stateGAME;
}

// The balls are freely movable sprites.
void doBalls(SDL_Surface* destinationBuffer)
{
	SDL_Rect blitDest;
	SDL_Rect areatoBlit;
	int currentlevOffset = 0;
	int curcol = 0;
	areatoBlit.w = tileWidth;
	areatoBlit.h = tileHeight;
	blitDest.x = 0;
	blitDest.y = 0;
	while (currentlevOffset < 176)
	{
		if (gameArea[currentlevOffset] != 0 && gameArea[currentlevOffset] < tiledefWall)
		{
			areatoBlit.x = tilesetXoffsets[gameArea[currentlevOffset]];
			areatoBlit.y = tilesetYoffsets[gameArea[currentlevOffset]];
			SDL_BlitSurface(tilegfx, &areatoBlit, destinationBuffer, &blitDest);
		}
		blitDest.x += tileWidth;
		curcol++;
		if (curcol >= tileColumns)
		{
			curcol = 0;
			blitDest.x = 0;
			blitDest.y += tileHeight;
		}
		currentlevOffset++;
	}
	if (movingballColour != 0)
	{
		blitDest.x = movingballX;
		blitDest.y = movingballY;
		areatoBlit.x = tilesetXoffsets[movingballColour];
		areatoBlit.y = tilesetYoffsets[movingballColour];
		SDL_BlitSurface(tilegfx, &areatoBlit, destinationBuffer, &blitDest);
	}
}

// Returns the exact offset on the game area array according to the given X,Y coordinates.
int getexactLocation(int checkx, int checky)
{
	return (checky * tileColumns ) + checkx;
}

// Reports which element is detected at the given location on the game area.
int blockatLocation(int checkx, int checky)
{
	int exactLocation = (checky * tileColumns) + checkx;
	return gameArea[exactLocation];
}

// The player can only move if there is either a ball next to him and said ball is free to move,
// or if there is empty space next to the player.
bool canmovethere(int checkx, int checky)
{
	if (blockatLocation(checkx, checky) == tiledefSpace)
	{
		return true;
	}
	// If there's a ball next to the player, the ball itself must be free to move.
	if (blockatLocation(checkx, checky) < tiledefWall)
	{
		movingballX = playerXpos;
		movingballY = playerYpos;
		switch (playerDir)
		{
		case dirUP:
			if (blockatLocation(checkx, (checky - 1)) != tiledefSpace || playerY < 2) return false;
			movingballY -= tileHeight;
			break;
		case dirDOWN:
			if (blockatLocation(checkx, (checky + 1)) != tiledefSpace || playerY > (tileLines - 2)) return false;
			movingballY += tileHeight;
			break;
		case dirLEFT:
			if (blockatLocation((checkx - 1), checky) != tiledefSpace || playerX < 2) return false;
			movingballX -= tileWidth;
			break;
		case dirRIGHT:
			if (blockatLocation((checkx + 1), checky) != tiledefSpace || playerX > (tileColumns - 2)) return false;
			movingballX += tileWidth;
			break;
		}
		int exactLocation = (checky * tileColumns) + checkx;
		movingballColour = gameArea[exactLocation];
		gameArea[exactLocation] = tiledefSpace;
		return true;
	}
	return false;
}

// Check the type of block and let the player move if possible.
void checkBlock(const char direction)
{
	int checkx = playerX;
	int checky = playerY;
	switch (direction)
	{
	case dirUP:
		checky = playerY - 1;
		if(canmovethere(checkx, checky) == true)
		{
			gameState = stateMOVING;
			movementUnits = 10;
			ymovementDelta = -objSpeed;
			playerY--;
		}
		break;
	case dirDOWN:
		checky = playerY + 1;
		if (canmovethere(checkx, checky) == true)
		{
			gameState = stateMOVING;
			movementUnits = 10;
			ymovementDelta = objSpeed;
			playerY++;
		}
		break;
	case dirLEFT:
		checkx = playerX - 1;
		if (canmovethere(checkx, checky) == true)
		{
			gameState = stateMOVING;
			movementUnits = 10;
			xmovementDelta = -objSpeed;
			playerX--;
		}
		break;
	case dirRIGHT:
		checkx = playerX + 1;
		if (canmovethere(checkx, checky) == true)
		{
			gameState = stateMOVING;
			movementUnits = 10;
			xmovementDelta = objSpeed;
			playerX++;
		}
		break;
	}
}

// Read the keyboard and set movement variables accordingly.
void readKeyboard()
{
	if (gameState == stateGAME)
	{
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		if (currentKeyStates[SDL_SCANCODE_UP] && playerY > 0 && gameState == stateGAME)
		{
			playerDir = dirUP;
			checkBlock(dirUP);
		}
		if (currentKeyStates[SDL_SCANCODE_DOWN] && playerY < (tileLines - 1) && gameState == stateGAME)
		{
			playerDir = dirDOWN;
			checkBlock(dirDOWN);
		}
		if (currentKeyStates[SDL_SCANCODE_LEFT] && playerX > 0 && gameState == stateGAME)
		{
			playerDir = dirLEFT;
			checkBlock(dirLEFT);
		}
		if (currentKeyStates[SDL_SCANCODE_RIGHT] && playerX < (tileColumns - 1) && gameState == stateGAME)
		{
			playerDir = dirRIGHT;
			checkBlock(dirRIGHT);
		}
		if (currentKeyStates[SDL_SCANCODE_R])
		{
			Mix_PlayChannel(-1, morphsnd2, 0);
			gameState = stateRETRY;
			SDL_Rect posi;
			posi.x = playerXpos;
			posi.y = playerYpos;
			doBalls(image3);
			SDL_BlitSurface(image1, NULL, image3, &posi);
			tryagainY = 590;
			tryagainH = 10;
			delay = 100;
		}
	}
}

// Return the absolute screen X coordinate from the given tile X coordinate.
int getgamescreenX(int checkx)
{
	return checkx * 50;
}
// Return the absolute screen Y coordinate from the given tile Y coordinate.
int getgamescreenY(int checky)
{
	return checky * 50;
}

// This is the most important in-game routine, the one that checks whether a ball
// is surrounded by other balls of the same colour. If it is, then said ball and
// all the balls around it will disappear.
// When a ball disappears, it is removed from the game area array and AnimObject
// location pointers are set to point to the location where the ball was.
void checkBalls(int checkx, int checky)
{
	int animIndex = 0;
	bool matchesFound = false;
	if (gameArea[getexactLocation(checkx, (checky - 1))] == movingballColour)
	{
		animObj[(animIndex * 2)] = getgamescreenX(checkx);
		animObj[((animIndex * 2) + 1)] = getgamescreenY(checky - 1);
		animObjindex[animIndex] = 0;
		animIndex++;
		gameArea[getexactLocation(checkx, (checky - 1))] = tiledefSpace;
		matchesFound = true;
	}
	if (gameArea[getexactLocation(checkx, (checky + 1))] == movingballColour)
	{
		animObj[(animIndex * 2)] = getgamescreenX(checkx);
		animObj[((animIndex * 2) + 1)] = getgamescreenY(checky + 1);
		animObjindex[animIndex] = 0;
		animIndex++;
		gameArea[getexactLocation(checkx, (checky + 1))] = tiledefSpace;
		matchesFound = true;
	}
	if (gameArea[getexactLocation((checkx - 1), checky)] == movingballColour)
	{
		animObj[(animIndex * 2)] = getgamescreenX(checkx - 1);
		animObj[((animIndex * 2) + 1)] = getgamescreenY(checky);
		animObjindex[animIndex] = 0;
		animIndex++;
		gameArea[getexactLocation((checkx - 1), checky)] = tiledefSpace;
		matchesFound = true;
	}
	if (gameArea[getexactLocation((checkx + 1), checky)] == movingballColour)
	{
		animObj[(animIndex * 2)] = getgamescreenX(checkx + 1);
		animObj[((animIndex * 2) + 1)] = getgamescreenY(checky);
		animObjindex[animIndex] = 0;
		animIndex++;
		gameArea[getexactLocation((checkx + 1), checky)] = tiledefSpace;
		matchesFound = true;
	}
	if (matchesFound == true)
	{
		Mix_PlayChannel(-1, morphsnd1, 0);
		gameArea[getexactLocation(checkx, checky)] = tiledefSpace;
		animObj[(animIndex * 2)] = getgamescreenX(checkx);
		animObj[((animIndex * 2) + 1)] = getgamescreenY(checky);
		animObjindex[animIndex] = 0;
		animIndex++;
	}
}

// You moved a ball, so now we check whether there are other balls of the same colour next to it.
void ballMoved()
{
	switch (playerDir)
	{
	case dirUP:
		gameArea[getexactLocation(playerX, (playerY - 1))] = movingballColour;
		checkBalls(playerX, (playerY - 1));
		break;
	case dirDOWN:
		gameArea[getexactLocation(playerX, (playerY + 1))] = movingballColour;
		checkBalls(playerX, (playerY + 1));
		break;
	case dirLEFT:
		gameArea[getexactLocation((playerX - 1), playerY)] = movingballColour;
		checkBalls((playerX - 1), playerY);
		break;
	case dirRIGHT:
		gameArea[getexactLocation((playerX + 1), playerY)] = movingballColour;
		checkBalls((playerX + 1), playerY);
		break;
	}
	movingballColour = 0;
}

// If there are any enabled animations, then we place the AnimObjects (animated gfx objects)
// at the X,Y locations that have been stored to the AnimObject location array.
void doAnims()
{
	SDL_Rect srcgfx;
	SDL_Rect dest;
	srcgfx.y = 0;
	srcgfx.w = 50;
	srcgfx.h = 50;
	dest.w = 50;
	dest.h = 50;
	// We go through all the AnimObjects.
	//int offset = 0;
	//while (offset < 10)
	for (int offset = 0; offset < 10; offset++)
	{
		if (animObj[(offset * 2)] >= 0)
		{
			srcgfx.x = animObjindex[offset] * 50;
			dest.x = animObj[(offset * 2)];
			dest.y = animObj[((offset * 2) + 1)];
			SDL_BlitSurface(gameAnimations, &srcgfx, image2, &dest);
			animObjindex[offset]++;
			if (animObjindex[offset] >= 10) animObj[(offset * 2)] = -1;
		}
	}
}

// At the title and ending screens, we use this buffered keyboard reader, so
// that a keyboard key is reported to be pressed when the user presses it, but
// not reported as held down. The other keyboard reader is more ideal for in-game.
void handleEvent(SDL_Event& e)
{
	if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
	{
		switch (e.key.keysym.sym)
		{
		case SDLK_SPACE:
			if (gameState == stateTITLE)
			{
				gameState = stateLEVELCOMPLETE;
				screentransitionInit();
				delay = 50;
				level = 0;
			}
			if (gameState == stateENDSCREEN)
			{
				gameState = stateTITLE;
				initTitle();
				SDL_BlitSurface(titlescreen, NULL, image3, NULL);
			}
			break;
		}
	}
}

// Scroll the scrolling text at the title page.
void scrollText()
{
	SDL_Rect src, dst;
	int pos, lettersHandled, nextcharinScrolltext;
	src.x = 0;
	src.y = 0;
	src.w = 100;
	src.h = 100;
	dst.x = 0;
	dst.y = scrolltextY;
	SDL_BlitSurface(titlescreen, NULL, image3, NULL);
	if (titlescrollOffset == 0)
	{
		pos = 0;
		lettersHandled = 0;
		while (lettersHandled < 8)
		{
			dst.x = pos;
			src.x = (scrolltextArray[lettersHandled] - 32) * 100;
			SDL_BlitSurface(gameFont, &src, image3, &dst);
			pos += 100;
			lettersHandled++;
		}
	}
	else
	{
		src.x = (scrolltextArray[0] - 32) * 100 + titlescrollOffset;
		src.w = 100 - titlescrollOffset;
		SDL_BlitSurface(gameFont, &src, image3, &dst);
		pos = 100 - titlescrollOffset;
		lettersHandled = 1;
		src.w = 100;
		while (lettersHandled < 8)
		{
			dst.x = pos;
			src.x = (scrolltextArray[lettersHandled] - 32) * 100;
			SDL_BlitSurface(gameFont, &src, image3, &dst);
			pos += 100;
			lettersHandled++;
		}
		dst.x = pos;
		src.w = titlescrollOffset;
		src.x = (scrolltextArray[lettersHandled] - 32) * 100;
		SDL_BlitSurface(gameFont, &src, image3, &dst);
	}
	titlescrollOffset += 5;
	if (titlescrollOffset >= 100)
	{
		nextcharinScrolltext = titleText.at(scrollingtextPos);
		scrolltextArray[0] = scrolltextArray[1];
		scrolltextArray[1] = scrolltextArray[2];
		scrolltextArray[2] = scrolltextArray[3];
		scrolltextArray[3] = scrolltextArray[4];
		scrolltextArray[4] = scrolltextArray[5];
		scrolltextArray[5] = scrolltextArray[6];
		scrolltextArray[6] = scrolltextArray[7];
		scrolltextArray[7] = scrolltextArray[8];
		scrolltextArray[8] = nextcharinScrolltext;
		scrollingtextPos++;
		if (scrollingtextPos >= titleText.length()) scrollingtextPos = 0;
		titlescrollOffset = 0;
	}
}

int main(int argc, char* args[])
{
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to load media!\n");
		}
		bool quit = false;
		SDL_Event e;
		SDL_Rect posi;
		SDL_SetColorKey(image1, 1, 0);
		SDL_SetColorKey(tilegfx, 1, 0);
		SDL_SetColorKey(gameAnimations, 1, 0);
		SDL_SetColorKey(gameFont, 1, 0);
		std::streampos size;
		std::ifstream levelfile("data/levels.dat", std::ios::in|std::ios::binary|std::ios::ate);
		if (levelfile.is_open())
		{
			size = levelfile.tellg();
			levelfile.seekg(0, std::ios::beg);
			levelfile.read(levelData, size);
			levelfile.close();
		}
		else
		{
			std::cout << "Error reading level data!";
		}
		gameState = stateTITLE;
		initTitle();
		noAnim();
		SDL_BlitSurface(titlescreen, NULL, image3, NULL);
		while (!quit)
		{
			if (gameState == stateTITLE)
			{
				scrollText();
			}
			if (gameState == stateGAME)
			{
				bool levelclear = true;
				for(int checkpos = 0; checkpos < 176 && levelclear == true; checkpos++)
				{
					if (gameArea[checkpos] != tiledefSpace && gameArea[checkpos] < tiledefWall) levelclear = false;
				}
				if (levelclear == true)
				{
					gameState = stateLEVELCOMPLETE;
					delay = 50;
					SDL_Rect posi;
					posi.x = playerXpos;
					posi.y = playerYpos;
					doBalls(image3);
					SDL_BlitSurface(image1, NULL, image3, &posi);
					screentransitionInit();
				}
			}
			if (gameState == stateRETRY)
			{
				SDL_Rect dimensions;
				dimensions.x = 0;
				dimensions.y = tryagainY;
				dimensions.w = 800;
				dimensions.h = tryagainH;
				if(tryagainY >= 0) SDL_BlitSurface(blankness, &dimensions, image3, NULL);
				if (tryagainY >= 0) tryagainY -= 10;
				if (tryagainH < 600) tryagainH += 10;
				if (tryagainY < 0)
				{
					if (delay == 0)
					{
						gameState = stateGAME;
						loadLevel();
					}
					else
					{
						delay--;
					}
				}
			}
			if (gameState == stateLEVELCOMPLETE)
			{
				screenTransition();
				if (screenTransitionPhase == 0)
				{
					if (delay > 0)
					{
						delay--;
					}
					else
					{
						level++;
						if (level < 31)
						{
							loadLevel();
						}
						else
						{
							gameState = stateENDSCREEN;
							SDL_BlitSurface(endscreen, NULL, image3, NULL);
						}
					}
				}
			}
			// Handle various events.
			while (SDL_PollEvent(&e) != 0)
			{
				if (e.type == SDL_QUIT)
				{
					quit = true;
				}
				handleEvent(e);
			}
			readKeyboard();
			if (gameState == stateGAME || gameState == stateMOVING)
			{
				// Move the player.
				if (movementUnits == 0)
				{
					if (movingballColour != 0) ballMoved();
					gameState = stateGAME;
					xmovementDelta = 0;
					ymovementDelta = 0;
				}
				else
				{
					playerXpos += xmovementDelta;
					playerYpos += ymovementDelta;
					movingballX += xmovementDelta;
					movingballY += ymovementDelta;
					movementUnits--;
				}
			}
			posi.x = playerXpos;
			posi.y = playerYpos;
			SDL_BlitSurface(image3, NULL, image2, NULL);
			if (gameState == stateGAME || gameState == stateMOVING)
			{
				doBalls(image2);
				SDL_BlitSurface(image1, NULL, image2, &posi);
			}
			doAnims();
			usableTexture = SDL_CreateTextureFromSurface(gRenderer, image2);
			SDL_RenderCopy(gRenderer, usableTexture, NULL, NULL);
			SDL_DestroyTexture(usableTexture);
			SDL_RenderPresent(gRenderer); // Refresh the game screen.
		}
	}
	close();
	return 0;
}