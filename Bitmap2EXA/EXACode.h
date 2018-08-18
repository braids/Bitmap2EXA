#pragma once
#include "stdafx.h"
#include "EXASprite.h"

// EXAs
#define EXA_DATA_LEN 20
#define EXA_LINE_LEN 24
#define EXA_MAX_LINES 100

struct EXAInstr {
public:
	char text[EXA_LINE_LEN*EXA_MAX_LINES];

	// Add string
	void Add(const char* source) {
		strcat(text, source);
	}
	void AddN(const char* source, int length) {
		strncat(text, source, EXA_DATA_LEN);
	}

	// COPY
	void AddCOPY(char* source, char* dest) {
		Add("COPY ");
		Add(source);
		Add(" ");
		Add(dest);
		AddNewline();
	}
	void AddCOPY(int num, char* dest) {
		char numstring[4]; numstring[0] = '\0';
		_itoa(num, numstring, BASE_10);
		AddCOPY(numstring, dest);
	}
	void AddCOPY_GP(char* source) {
		AddCOPY(source, (char*)"GP");
	}
	void AddCOPY_GP(int num) {
		char numstring[4]; numstring[0] = '\0';
		_itoa(num, numstring, BASE_10);
		AddCOPY_GP(numstring);
	}
	void AddCOPY_GX(char* source) {
		AddCOPY(source, (char*)"GX");
	}
	void AddCOPY_GX(int num) {
		char numstring[4]; numstring[0] = '\0';
		_itoa(num, numstring, BASE_10);
		AddCOPY_GX(numstring);
	}
	void AddCOPY_GY(char* source) {
		AddCOPY(source, (char*)"GY");
	}
	void AddCOPY_GY(int num) {
		char numstring[4]; numstring[0] = '\0';
		_itoa(num, numstring, BASE_10);
		AddCOPY_GY(numstring);
	}

	// DATA
	void AddDATA(char* data) {
		int remaininglength;

		for (int j = 0; j < strlen(data); j += EXA_DATA_LEN) {
			// Get remaining length until end of string
			remaininglength = strlen(&data[j]);

			Add("DATA ");

			// If less than max data string left, grab what's left and move to end
			if (remaininglength <= 20)
				Add(&data[j]); 
			// Add instructions to string
			else
				AddN(&data[j], EXA_DATA_LEN);

			AddNewline();
		}
	}

	// JUMP
	void AddJUMP(char* mark) {
		Add("JUMP ");
		Add(mark);
		AddNewline();
	}
	void AddFJMP(char* mark) {
		Add("FJMP ");
		Add(mark);
		AddNewline();
	}
	void AddTJMP(char* mark) {
		Add("TJMP ");
		Add(mark);
		AddNewline();
	}

	// MARK
	void AddMARK(char* mark) {
		Add("MARK ");
		Add(mark);
		AddNewline();
	}

	// LINK
	void AddLINK(char* link) {
		Add("LINK ");
		Add(link);
		AddNewline();
	}
	void AddLINK(int link) {
		char numstring[4]; numstring[0] = '\0';
		_itoa(link, numstring, BASE_10);
		AddLINK(numstring);
	}
	
	// Newline
	void AddNewline(int n) {
		for (int i = 0; i < n; i++)
			Add("\n");
	}
	void AddNewline() {
		AddNewline(1);
	}

	void AddTEST(char* test) {
		Add("TEST ");
		Add(test);
		AddNewline();
	}

};

class EXACode {
public:
	static void PrintGPData(EXASpriteList& spriteInstrList, int offsetX, int offsetY) {
		EXAInstr EXABuffer[MAX_SPRITES];
		for (int i = 0; i < MAX_SPRITES; i++) EXABuffer[i].text[0] = '\0';
		EXAInstr* EXAHead;
		EXASpriteInstr* InstrHead;

		// Convert instructions into EXA code
		for (int i = 0; i < spriteInstrList.length; i++) {
			// Get data pointers
			EXAHead = &EXABuffer[i];
			InstrHead = &spriteInstrList.list[i];

			// Add Data to file
			EXAHead->AddDATA(spriteInstrList.list[i].text);
			EXAHead->AddNewline();
			
			// Move and position
			EXAHead->AddLINK(800);
			EXAHead->AddCOPY_GX(offsetX + (InstrHead->x * SPRITE_W));
			EXAHead->AddCOPY_GY(offsetY + (InstrHead->y * SPRITE_H));
			EXAHead->AddNewline();
			
			// Load data into GP
			EXAHead->Add("MARK LOAD\nCOPY F GP\nTEST EOF\nFJMP LOAD\n\n");

			// Start loop
			EXAHead->Add("MARK LOOP\nWAIT\nJUMP LOOP\n\n");
		}

		// Print code to screen
		printf("======== BMP2EXA ========\n\n");
		for (int i = 0; i < spriteInstrList.length; i++) {
			printf("========== %2d ==========\n\n", i);
			printf(EXABuffer[i].text);
		}
		printf("=========================\n\n");
	}
};
