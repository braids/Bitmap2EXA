// Bitmap2EXA.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "BitArray.h"
#include "EXAHeaders.h"
#include <cassert>

// Arguments
#define ARG_FILE 1
#define ARG_XPOS 2
#define ARG_YPOS 3

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
	EXASpriteMap spritemap(bitarray);

	// Get list of sprites to draw
	EXASpriteList litsprites = spritemap.GetInstrList();

	// Print sprite-drawin' EXA code
	EXACode::PrintGPData(litsprites, offsetX, offsetY);
}
