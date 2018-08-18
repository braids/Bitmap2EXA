#pragma once
#include "stdafx.h"
#include <cassert>

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
	BitArray bitarray = { width, height };

	// Get bit data
	bitarray.length = fread(bitarray.data, sizeof(unsigned char), size, SourceBMP);
	fclose(SourceBMP);

	// Return bit array
	return bitarray;

}
