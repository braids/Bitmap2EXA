#pragma once
#include "stdafx.h"
#include <cstring>
#include <cmath>
#include "BitArray.h"

// Math
#define BASE_10 10

// Sprites
#define SPRITE_W 10
#define SPRITE_H 10
#define MAX_SPRITES_W 12
#define MAX_SPRITES_H 10
#define MAX_SPRITES MAX_SPRITES_W * MAX_SPRITES_H
#define GETSPRITENUM(X) ((int) (X / SPRITE_W))

#define INSTR_BUFFER 11 * 11 * 4

struct EXASprite;
struct EXASpriteList;
struct EXASpriteMap;

struct EXASprite {
public:
	int x, y;
	bool booldata[SPRITE_W][SPRITE_H];
	char instructions[INSTR_BUFFER];

	EXASprite() {
		for (int y = 0; y < SPRITE_W; y++)
			for (int x = 0; x < SPRITE_H; x++)
				booldata[x][y] = false;
	}

	void SetPos(int _x, int _y) {
		x = _x;
		y = _y;
	}

	void SetPixelFromMapPos(int px, int py, bool value) {
		// Convert map coord to local coord, set flag
		booldata[px % SPRITE_W][py % SPRITE_H] = value;
	}

	void GenInstructions() {
		char instrBuffer[5];
		// Position buffer for X/Y value strings
		char posBuffer[2];

		instructions[0] = '\0';

		// Check for and write down lit pixels
		for (int py = 0; py < SPRITE_H; py++) {
			for (int px = 0; px < SPRITE_W; px++) {
				// If pixel is lit
				if (booldata[px][py]) {
					instrBuffer[0] = '\0';

					// Turn on pixel
					strcat(instrBuffer, "1");

					// Get X pos
					posBuffer[0] = '\0';
					_itoa(px, posBuffer, BASE_10);
					strcat(instrBuffer, posBuffer);

					// Get Y pos
					posBuffer[0] = '\0';
					_itoa(py, posBuffer, BASE_10);
					strcat(instrBuffer, posBuffer);

					// Add spacer
					strcat(instrBuffer, " ");

					// Append to instructions
					strcat(instructions, instrBuffer);
				}
			}
		}
	}
};

struct EXASpriteInstr {
public:
	int x, y;
	char text[INSTR_BUFFER];
};

struct EXASpriteList {
public:
	int length;
	EXASpriteInstr list[MAX_SPRITES];

	EXASpriteList(int _length, EXASpriteInstr* _list) :
		length(_length)
	{
		for (int i = 0; i < length; i++)
			list[i] = _list[i];
	}
};

struct EXASpriteMap {
	int width = 0, height = 0;
	EXASprite map[MAX_SPRITES_W][MAX_SPRITES_H];
	bool booldata[BMP_MAX_W*BMP_MAX_H];
	int booldatasize = 0;

	EXASpriteMap(BitArray ba) {
		EXASprite* headsprite;
		unsigned char currentbyte;
		bool pixel;
		int colOffset = 0, rowOffset = 0;

		// Set width and height
		width = GETSPRITENUM(ba.width) + 1;
		height = GETSPRITENUM(ba.height) + 1;

		// Init bool data
		for (int i = 0; i < BMP_MAX_W*BMP_MAX_H; i++) booldata[i] = false;

		// Check each byte in BMP data
		for (int i = 0; i < ba.length; i++) {
			// If padding bytes found, discard
			if (i % (width + BMP_PADDING_SIZE) > width) {
				colOffset++;
				continue;
			}

			//iterate thru each byte in char
			for (int b = 7; b >= 0; b--) {
				// Get the bit in the current row with column offset applied
				int rowbit = (7 - b) + ((i - colOffset) * 8);

				// If past the width margin, add to row offset and continue
				if ((rowbit % (ba.width + BMP_PADDING_SIZE + 1)) >= ba.width) {
					rowOffset++;
					continue;
				}

				// Subtract row offset
				rowbit -= rowOffset;

				// set bool data if char data bit is 1 or 0
				currentbyte = ba.data[i];
				booldata[rowbit] = !((currentbyte >> b) & 0x01);

				// Increment bool count, bail if max size reached
				booldatasize++;
				if (booldatasize >= BMP_MAX_W * BMP_MAX_H)
					break;
			}
			if (booldatasize >= BMP_MAX_W * BMP_MAX_H)
				break;
		}

		// Init sprite maps
		for (int y = 0; y < height; y++)
			for (int x = 0; x < width; x++)
				map[x][y];

		// Copy BMP data into sprite map
		for (int y = 0; y < ba.height; y++) {
			for (int x = 0; x < ba.width; x++) {
				// Get sprite at xy position
				headsprite = &map[GETSPRITENUM(x)][GETSPRITENUM(y)];

				// Get BMP pixel at xy location. Reverse Y loading.
				pixel = booldata[x + ((ba.height * ba.width) - ((y + 1) * ba.width))];

				// Set sprite pixel at xy location
				headsprite->SetPixelFromMapPos(x, y, pixel);
			}
		}

		for (int i = 0; i < width * height; i++) {
			int x = i % width, y = (int)std::floor((double)i / (double)width);
			map[x][y].SetPos(x, y);
			map[x][y].GenInstructions();
		}
	}

	EXASpriteList GetInstrList() {
		EXASpriteInstr spriteInstrBuffer[MAX_SPRITES];

		int numspriteslit = 0;

		// Check for sprites that generated instructions
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				// If instructions found, add to sprite buffer
				if (map[x][y].instructions[0] == '1') {
					spriteInstrBuffer[numspriteslit] = { x, y };
					strcpy(spriteInstrBuffer[numspriteslit].text, map[x][y].instructions);
					numspriteslit++;
				}
			}
		}

		// Return sprites with at least one lit pixel
		return EXASpriteList(numspriteslit, spriteInstrBuffer);
	}
};
