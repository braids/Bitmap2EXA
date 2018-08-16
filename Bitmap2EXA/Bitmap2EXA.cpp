// Bitmap2EXA.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdint.h>

// Arguments
#define ARG_FILE 1
#define ARG_XPOS 2
#define ARG_YPOS 3

// BMPs
#define BMP_MAX_W 120
#define BMP_MAX_H 100

#define BMP_HEADER_LENGTH 54
#define BMP_HEADER_OFFSET 10
#define BMP_HEADER_W 18
#define BMP_HEADER_H 22
#define BMP_HEADER_BPP 28

#define BMP_BPP_1 0x01

#define BMP_PADDING_SIZE 3

#define BMP_PALETTE 8

// EXAs
#define EXA_DATA_LEN 20

// Math
#define BASE_10 10

// Sprites
#define SPRITE_W 10
#define SPRITE_H 10
#define MAX_SPRITES_W 12
#define MAX_SPRITES_H 10
#define MAX_SPRITES MAX_SPRITES_W * MAX_SPRITES_H
#define GETSPRITENUM(X) ((int) (X / SPRITE_W))

struct BitArray {
	int width, height, length;
	unsigned char data[BMP_MAX_W * BMP_MAX_H];
};

BitArray GetBMPBitArray(char* filename) {
	int width = 0, height = 0, size = 0;
	
	unsigned char header[BMP_HEADER_LENGTH];
	
	// Open BMP and get header information
	FILE* SourceBMP = fopen(filename, "rb");
	
	assert(SourceBMP != nullptr && "File didn't load.");

	fread(header, sizeof(unsigned char), BMP_HEADER_LENGTH, SourceBMP);
	fseek(SourceBMP, BMP_PALETTE, SEEK_CUR);
	
	// Bail if BMP isn't monochrome
	assert(*(unsigned char*)&header[BMP_HEADER_BPP] == BMP_BPP_1 && "BMP must be monochrome.");

	// Get BMP width/height/size
	width = *(int*)&header[BMP_HEADER_W];
	height = *(int*)&header[BMP_HEADER_H];
	size = width * height;

	// Create bit array
	BitArray bitarray = { width, height};

	// Get bit data
	bitarray.length = fread(bitarray.data, sizeof(unsigned char), size, SourceBMP);
	fclose(SourceBMP);

	// Return bit array
	return bitarray;
	
}

struct Sprite {
	int x, y;
	bool booldata[SPRITE_W][SPRITE_H];
	char instructions[11*11*4];

	Sprite() {
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

struct SpriteList {
	int length;
	Sprite** list;

	SpriteList(int _length, Sprite** _list) : 
		length(_length), 
		list(_list) 
	{}
};

struct SpriteMap {
	int width = 0, height = 0;
	Sprite map[MAX_SPRITES_W][MAX_SPRITES_H];
	bool booldata[BMP_MAX_W*BMP_MAX_H];
	int booldatasize = 0;

	SpriteMap(BitArray ba) {
		Sprite* headsprite;
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

			// std::floor((double)i / (double)ba.width) >= 1.0

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
				//printf("CurrentByte:%c BoolData:%d i:%d b:%d Bit:%d BAWidth:%d\n", currentbyte, rowbit, i, b, currentbyte >> b, ba.width);
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
				//printf("x:%d y:%d pixel:%d BDLoc:%d\n", x, y, pixel, x + (y * ba.width));
				headsprite->SetPixelFromMapPos(x, y, pixel);
			}
		}

		for (int i = 0; i < width * height; i++) {
			int x = i % width, y = (int)std::floor((double)i / (double)width);
			map[x][y].SetPos(x, y);
			map[x][y].GenInstructions();
		}
	}

	SpriteList GetLitSpriteList() {
		Sprite* spriteBuffer[MAX_SPRITES];

		int numspriteslit = 0;
		
		// Check for sprites that generated instructions
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				// If instructions found, add to sprite buffer
				if (map[x][y].instructions[0] == '1')
					spriteBuffer[numspriteslit++] = &map[x][y];
			}
		}

		// Return sprites with at least one lit pixel
		return SpriteList(numspriteslit, spriteBuffer);
	}
};

int main(int argc, char* argv[])
{
	int offsetX = 0, offsetY = 0;

	// Do some input sanity checks
	assert(argc > 1 && "Please provide a Monochrome BMP file.");
	
	if (argc <= 3) 
		printf("Using default X/Y location of (0,0).");
	else {
		offsetX = atoi(argv[ARG_XPOS]);
		offsetY = atoi(argv[ARG_YPOS]);
	}

	// Create bit array from monochrome BMP file
	BitArray bitarray = GetBMPBitArray(argv[ARG_FILE]);	

	// Create sprite map from bit array
	SpriteMap spritemap(bitarray);
	
	// Get list of sprites to draw
	SpriteList litsprites = spritemap.GetLitSpriteList();

	// Instruction buffer for EXAs
	char EXAbuffer[MAX_SPRITES][1024];
	for (int i = 0; i < MAX_SPRITES; i++)EXAbuffer[i][0] = '\0';
	char* EXAhead;
	char stringbuffer[1024]; stringbuffer[0] = '\0';
	Sprite* spritehead;
	int ilen;

	// Generate EXA-formatted code
	for (int i = 0; i < litsprites.length; i++) {
		EXAhead = EXAbuffer[i];
		spritehead = litsprites.list[i];
		ilen = strlen((char*)spritehead->instructions);

		// Write image data
		for (int j = 0; j < strlen(spritehead->instructions); j += EXA_DATA_LEN) {
			// Get remaining length until end of string
			int remaininglength = strlen(&spritehead->instructions[j]);
			
			strcat(EXAhead, "DATA ");
			
			// If less than max data string left, grab what's left and move to end
			if (remaininglength <= 20) {
				strncat(EXAhead, &spritehead->instructions[j], remaininglength);
				j += remaininglength;
			}
			// Add instructions to string
			else 
				strncat(EXAhead, &spritehead->instructions[j], EXA_DATA_LEN);
			
			strcat(EXAhead, "\n");
		}

		strcat(EXAhead, "\n");

		// Movement and positioning
		strcat(EXAhead, "LINK 800\n");
		
		strcat(EXAhead, "COPY ");
		_itoa(offsetX + (spritehead->x * SPRITE_W), stringbuffer, BASE_10);
		strcat(EXAhead, stringbuffer);
		strcat(EXAhead, " GX\n");

		strcat(EXAhead, "COPY ");
		_itoa(offsetX + (spritehead->y * SPRITE_H), stringbuffer, BASE_10);
		strcat(EXAhead, stringbuffer);
		strcat(EXAhead, " GY\n\n");

		// Load data into GP
		strcat(EXAhead, "MARK LOAD\nCOPY F GP\nTEST EOF\nFJMP LOAD\n\n");

		// Start loop
		strcat(EXAhead, "MARK LOOP\nWAIT\nJUMP LOOP\n\n");
	}

	// Print code to screen
	printf("======== BMP2EXA ========\n\n");
	for (int i = 0; i < litsprites.length; i++) {
		printf("========== %2d ==========\n\n", i);
		printf(EXAbuffer[i]);
	}
	printf("=========================\n\n");
}

