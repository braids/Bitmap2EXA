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
	bool* booldatahead = booldata;
	int booldatasize = 0;

	EXASpriteMap(BitArray ba) {
		EXASprite* headsprite;
		bool pixel;
		int colOffset = 0, rowOffset = 0;

		// Set sprite map width and height
		width = GETSPRITENUM(ba.width) + 1;
		height = GETSPRITENUM(ba.height) + 1;

		// Init bool data
		for (int i = 0; i < BMP_MAX_W*BMP_MAX_H; i++) booldata[i] = false;

		//// NOTE: 
		//// - 4 chars to each row block
		//// - # of bits in row of BMP data will be (x * 32) width where x = # of row blocks
		//// - # of row blocks = (image_width / 32) + 1 | Implemented as Raw_Length()

		//// Future work: Make the below stuff prettier.

		// Read in data a row at a time
		for (int row = 0; row < ba.height; row++) {
			printf("Row:%d\n", row);

			// Read each block in BitArray
			for (int block = 0; block < ba.BlocksWidth(); block++) {
				printf("CurrentBlock:%d\n", block);
				
				// Read each char in the block (4 chars to each block)
				for (int blockchar = 0; blockchar < BMP_BLOCK_CHAR_SIZE; blockchar++) {

					// Read each bit in the char
					for (int bit = 7; bit >= 0; bit--) {

						// Get location of char in source data
						int datalocation = (row * BMP_BLOCK_CHAR_SIZE * ba.BlocksWidth()) + (block * BMP_BLOCK_CHAR_SIZE) + blockchar;

						// Get location of bit in destination data
						int booldatalocation = (row * BMP_BLOCK_BIT_SIZE * ba.BlocksWidth()) + (block * BMP_BLOCK_BIT_SIZE) + (blockchar * 8) + (7 - bit);

						// Break if max width reached (destination bit position % block bit width greater than width)
						if (booldatalocation % (ba.BlocksWidth() * BMP_BLOCK_BIT_SIZE) >= ba.width) {
							// Reset loops (break kills the whole thing)
							bit = 0;
							blockchar = BMP_BLOCK_CHAR_SIZE;
							block = ba.BlocksWidth();
							continue;
						}

						// Set destination bit true if source bit is false
						booldata[booldatalocation] = !((ba.data[datalocation] >> bit) & 0x01);
					}
				}
			}
		}

		// Copy BMP data into sprite map
		for (int y = 0; y < ba.height; y++) {
			for (int x = 0; x < ba.width; x++) {
				// Get sprite at xy position
				headsprite = &map[GETSPRITENUM(x)][GETSPRITENUM(y)];

				// Get pixel information from bool data array
				pixel = booldata[x + ((ba.height - y - 1) * ba.BitWidth())];

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
