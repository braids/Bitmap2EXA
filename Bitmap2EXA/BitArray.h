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

#define BMP_BLOCK_CHAR_SIZE 4
#define BMP_BLOCK_BIT_SIZE (BMP_BLOCK_CHAR_SIZE * 8)

struct BitArray {
	// Bit size information
	int width, height, length;

	// Char information
	unsigned char data[BMP_MAX_W * BMP_MAX_H];
	unsigned char block[BMP_BLOCK_CHAR_SIZE];

	// Get dimension info in blocks
	int BlocksWidth() {
		return (width / BMP_BLOCK_BIT_SIZE) + 1;
	}
	int BlocksHeight() {
		return (height / BMP_BLOCK_BIT_SIZE) + 1;
	}
	int BlocksTotal() {
		return BlocksWidth() * BlocksHeight();
	}

	// Get dimension info in bits
	int BitWidth() {
		return BlocksWidth() * BMP_BLOCK_BIT_SIZE;
	}

	// Retrn char data from specified block
	unsigned char* ReadBlock(int blocknum) {
		for (int i = 0; i < BMP_BLOCK_CHAR_SIZE; i++)
			block[i] = data[i + (blocknum * BMP_BLOCK_CHAR_SIZE)];

		return block;
	}
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
