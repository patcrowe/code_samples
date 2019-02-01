/*
Patrick Crowe
<patcrowe@umich.edu>
EECS 370

This program is a simulated linker for the LC2K computer designed for EECS 370.
The input is multiple .obj files for LC2K.
This program combines the input .obj files into one complete executable machine code program.

*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAXSIZE 300
#define MAXLINELENGTH 1000
#define MAXFILES 6

typedef struct FileData FileData;
typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct RelocationTableEntry RelocationTableEntry;
typedef struct CombinedFiles CombinedFiles;

struct SymbolTableEntry {
	char label[7];
	char location;
	int offset;
};

struct RelocationTableEntry {
	int offset;
	char inst[7];
	char label[7];
	int file;
};

struct FileData {
	int textSize;
	int dataSize;
	int symbolTableSize;
	int relocationTableSize;
	int textStartingLine; // in final executible
	int dataStartingLine; // in final executible
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry symbolTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
};

struct CombinedFiles {
	int text[MAXSIZE];
	int data[MAXSIZE];
	SymbolTableEntry     symTable[MAXSIZE];
	RelocationTableEntry relocTable[MAXSIZE];
	int textSize;
	int dataSize;
	int symTableSize;
	int relocTableSize;
};

bool isGlobal(char lab[]){
		return (lab[0] >= 'A' && lab[0] <= 'Z');
}

int isDefined(FileData *current_file, char lab[]){
	for (int i = 0; i < current_file->symbolTableSize; i++){
		if (!strcmp(current_file->symbolTable[i].label, lab) && current_file->symbolTable[i].location != 'U'){
			return i;
		}
	}
	return -1;
}

int main(int argc, char *argv[])
{
	char *inFileString, *outFileString;
	FILE *inFilePtr, *outFilePtr; 
	int i, j;

	if (argc <= 2) {
		printf("error: usage: %s <obj file> ... <output-exe-file>\n",
				argv[0]);
		exit(1);
	}

	outFileString = argv[argc - 1];

	outFilePtr = fopen(outFileString, "w");
	if (outFilePtr == NULL) {
		printf("error in opening %s\n", outFileString);
		exit(1);
	}

	FileData files[MAXFILES];

	//**************************~Start of code given to the class by instructors~***************************


	//Reads in all files and combines into master
	for (i = 0; i < argc - 2; i++) {
		inFileString = argv[i+1];

		inFilePtr = fopen(inFileString, "r");
		printf("opening %s\n", inFileString);

		if (inFilePtr == NULL) {
			printf("error in opening %s\n", inFileString);
			exit(1);
		}

		char line[MAXLINELENGTH];
		int sizeText, sizeData, sizeSymbol, sizeReloc;

		// parse first line
		fgets(line, MAXSIZE, inFilePtr);
		sscanf(line, "%d %d %d %d",
				&sizeText, &sizeData, &sizeSymbol, &sizeReloc);

		files[i].textSize = sizeText;
		files[i].dataSize = sizeData;
		files[i].symbolTableSize = sizeSymbol;
		files[i].relocationTableSize = sizeReloc;

		// read in text
		int instr;
		for (j = 0; j < sizeText; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			instr = atoi(line);
			files[i].text[j] = instr;
		}

		// read in data
		int data;
		for (j = 0; j < sizeData; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			data = atoi(line);
			files[i].data[j] = data;
		}

		// read in the symbol table
		char label[7];
		char type;
		int addr;
		for (j = 0; j < sizeSymbol; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%s %c %d",
					label, &type, &addr);
			files[i].symbolTable[j].offset = addr;
			strcpy(files[i].symbolTable[j].label, label);
			files[i].symbolTable[j].location = type;
		}

		// read in relocation table
		char opcode[7];
		for (j = 0; j < sizeReloc; j++) {
			fgets(line, MAXLINELENGTH, inFilePtr);
			sscanf(line, "%d %s %s",
					&addr, opcode, label);
			files[i].relocTable[j].offset = addr;
			strcpy(files[i].relocTable[j].inst, opcode);
			strcpy(files[i].relocTable[j].label, label);
			files[i].relocTable[j].file	= i;
		}
		fclose(inFilePtr);
	} // end reading files

	//**********************************~End of code given by instructors~***********************************

	int num_files = argc - 2;
	int totalSize = 0;

	for (int i = 0; i < num_files; i++){
		if (isDefined(&files[i], "Stack") >= 0) exit(1);
		totalSize += files[i].textSize;
		totalSize += files[i].dataSize;

		for (int j = 0; j < files[i].symbolTableSize; j++){
			for (int k = 0; k < num_files; k++){
				if (i != k && isDefined(&files[i], files[i].symbolTable[j].label) >= 0){
					//printf("exit");
					if (isDefined(&files[k], files[i].symbolTable[j].label) >= 0) exit(1);
					//printf("exit");
				}
			}
		}
	}

	int new_offset = 0;
	int mask = 0x0000FFFF;
	int old_offset = 0;
	for (int i = 0; i < num_files; i++){
		for (int j = 0; j < files[i].relocationTableSize; j++){
			new_offset = 0;
			old_offset = 0;

			// text local and Global(localy defined)
			if (strcmp(files[i].relocTable[j].inst, ".fill") && (!isGlobal(files[i].relocTable[j].label)
					|| (isGlobal(files[i].relocTable[j].label) && isDefined(&files[i], files[i].relocTable[j].label) >= 0))) {

				old_offset = (files[i].text[files[i].relocTable[j].offset]) & mask;
				//printf("%i\n", old_offset);

				// checking for use and declaration both in text
				if (old_offset >= files[i].textSize){
					for (int k = i + 1; k < num_files; k++){
						new_offset += files[k].textSize;
						//printf("check 1");
						if (isGlobal(files[i].relocTable[j].label) && isDefined(&files[k], files[i].relocTable[j].label) >= 0) exit(1);
						//printf("check 1");
					}
				}
				if (i != 0){
					for (int k = 0; k < i; k++){
						if (old_offset >= files[i].textSize){
							new_offset += files[k].dataSize;
						}
						new_offset += files[k].textSize;
						//printf("check 2");
						if (isGlobal(files[i].relocTable[j].label) && isDefined(&files[k], files[i].relocTable[j].label) >= 0) exit(1);
						//printf("check 2");
					}
				}
				files[i].text[files[i].relocTable[j].offset] += new_offset;
				//printf("%i \n", files[i].text[files[i].relocTable[j].offset]);
			}

			// text Global(not locally defined)
			else if (strcmp(files[i].relocTable[j].inst, ".fill")){
				if (!strcmp(files[i].relocTable[j].label, "Stack")){
					files[i].text[files[i].relocTable[j].offset] += totalSize;
				}
				else{
					int stLine = -1;
					int fileNum = 0;
					for (int k = 0; k < num_files; k++){
						//printf("check 3");
						if (stLine != -1 && isDefined(&files[k], files[i].relocTable[j].label) != -1) exit(1);
						//printf("check 3");
						if (stLine == -1){
							stLine = isDefined(&files[k], files[i].relocTable[j].label);
							fileNum = k;
						}
						
					}
					if (stLine == -1) exit(1);

					if (files[fileNum].symbolTable[stLine].location == 'D'){
						for (int k = 0; k < num_files; k++){
							new_offset += files[k].textSize;
						}
						if (fileNum != 0){
							for (int k = 0; k < fileNum; k++){
								new_offset += files[k].dataSize;
							}
						}
						new_offset += files[fileNum].symbolTable[stLine].offset;

						files[i].text[files[i].relocTable[j].offset] += new_offset;
					}
					else {
						if (fileNum != 0){
							for (int k = 0; k < fileNum; k++){
								new_offset += files[k].textSize;
							}
						}
						new_offset += files[fileNum].symbolTable[stLine].offset;

						files[i].text[files[i].relocTable[j].offset] += new_offset;
					}
				}
			}

			// data local and Global(locally defined)
			else if ((!isGlobal(files[i].relocTable[j].label)
					|| (isGlobal(files[i].relocTable[j].label) && isDefined(&files[i], files[i].relocTable[j].label) >= 0))){

				old_offset = (files[i].data[files[i].relocTable[j].offset]) & mask;

				for (int k = i + 1; k < num_files; k++){
					if (old_offset >= files[i].textSize){
						new_offset += files[k].textSize;
					}
					//printf("check 4");
					if (isGlobal(files[i].relocTable[j].label) && isDefined(&files[k], files[i].relocTable[j].label) >= 0) exit(1);
					//printf("check 4");
				}
				if (i != 0){
					for (int k = 0; k < i; k++){
						new_offset += files[k].textSize;
						if (old_offset >= files[i].textSize){
							new_offset += files[k].dataSize;
						}
						//printf("check 5");
						if (isGlobal(files[i].relocTable[j].label) && isDefined(&files[k], files[i].relocTable[j].label) >= 0) exit(1);
						//printf("check 5");
					}
				}

				files[i].data[files[i].relocTable[j].offset] += new_offset;
				//printf("%i \n", files[i].data[files[i].relocTable[j].offset]);
			}

			// data Global(not locally defined) 
			else {
				if (!strcmp(files[i].relocTable[j].label, "Stack")){
					files[i].data[files[i].relocTable[j].offset] += totalSize;
				}
				else{
					int stLine = -1;
					int fileNum = 0;
					for (int k = 0; k < num_files; k++){
						if (stLine != -1 && isDefined(&files[k], files[i].relocTable[j].label) != -1) exit(1);
						if (stLine == -1){
							stLine = isDefined(&files[k], files[i].relocTable[j].label);
							fileNum = k;
						}
						
					}
					//printf("check 2");
					if (stLine == -1) exit(1);

					if (files[fileNum].symbolTable[stLine].location == 'D'){
						for (int k = 0; k < num_files; k++){
							new_offset += files[k].textSize;
						}
						if (fileNum != 0){
							for (int k = 0; k < fileNum; k++){
								new_offset += files[k].dataSize;
							}
						}
						new_offset += files[fileNum].symbolTable[stLine].offset;

						files[i].data[files[i].relocTable[j].offset] += new_offset;
					}
					else {
						if (fileNum != 0){
							for (int k = 0; k < fileNum; k++){
								new_offset += files[k].textSize;
							}
						}
						new_offset += files[fileNum].symbolTable[stLine].offset;

						files[i].data[files[i].relocTable[j].offset] += new_offset;
					}
				}
			}
		}
	}

	for (int i = 0; i < num_files; i++){
		for (int j = 0; j < files[i].textSize; j++){
			fprintf(outFilePtr, "%i\n", files[i].text[j]);
			printf("%i\n", files[i].text[j]);
		}
	}
	for (int i = 0; i < num_files; i++){
		for (int j = 0; j < files[i].dataSize; j++){
			fprintf(outFilePtr, "%i\n", files[i].data[j]);
			printf("%i\n", files[i].data[j]);
		}
	}
} // end main