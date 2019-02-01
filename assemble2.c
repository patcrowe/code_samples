/* 
EECS 370
Patrick Crowe
<patcrowe@umich.edu>

This program takes in assembly code files using a limited language called
LC2K designed for the UMich course EECS 370 (Computer Organization) as input.
It then reads through the inrtuctions and data to create object files for
each input file with the corresponding machine code, symbol table, and 
relocation table.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAXLINELENGTH 1000


int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);
int argtoi(char *, char *);
int doSymbol(char *, char, int);
int doReloc(char *, char *, int);

struct Label{
	char *name;
	int address;
};

struct Symbol{
    char *name;
    char type;
    int os;
};

struct Relocate{
    int os;
    char *type;
    char *name;
};

static struct Relocate relocs[65536];
int num_relocs;

static struct Symbol symbols[35536];
int num_symbols;

static struct Label labels[65536];
int num_labels;

int 
main(int argc, char *argv[])
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }
    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    /*First run through the assembly to catch all of the labels and store them*/
    int instructions = 0;
    int fills = 0;
    int line_num = 0;
    int Globals = 0;
    int symbolics = 0;
    bool resolved = false;
    bool fillstart = false;
    num_labels = 0;
    num_symbols = 0;
    num_relocs = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        
    	if (label[0] != '\0' && label[0] != ' '){
    		if (num_labels > 0){
    			for (int i = 0; i < num_labels; i++){
    				if(strcmp(label, labels[i].name) == 0){
    					exit(1);
    				}
    			}
    		}
    		struct Label temp;
    		temp.name = (char*)malloc(sizeof(char) * 6);
    		strcpy(temp.name, label);
    		temp.address = line_num;
    		labels[num_labels] = temp;
    		num_labels++;
    	}
    	line_num++;
    }
    rewind(inFilePtr);
    line_num = 0;

    while(readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
        /*if(num_symbols > 2) printf("%s %s\n", symbols[0].name, symbols[1].name);*/
        
        resolved = false;

        if (!strcmp(opcode, ".fill")){
            fillstart = true;
            fills++;

            if (label[0] >= 'A' && label[0] <= 'Z'){
                doSymbol(label, 'D', (line_num - instructions)); 
                Globals++;
            }

            if ((arg0[0] >= 'A' && arg0[0] <= 'Z') && !isNumber(arg0)){
                for (int i = 0; i < num_labels; i++){
                    if (!strcmp(arg0, labels[i].name)) {
                        resolved = true;
                    }
                }
                if (resolved == false) {
                    doSymbol(arg0, 'U', 0);
                    Globals++;
                }
            }
            if (!isNumber(arg0)){
                symbolics++;
            }
        }
        else {
            if(opcode[0] != '\0'){
                instructions++;
            }
            if (fillstart == true) {
                exit(1);
            }
            if (label[0] >= 'A' && label[0] <= 'Z'){
                doSymbol(label, 'T', (line_num));
                Globals++; 
            }

            if (!strcmp(opcode, "lw") || !strcmp(opcode, "sw")) {
                if (arg2[0] >= 'A' && arg2[0] <= 'Z'){
                    if (arg2[0] >= 'A' && arg2[0] <= 'Z' && !isNumber(arg2)){
                        for (int i = 0; i < num_labels; i++){
                            if (!strcmp(arg2, labels[i].name)) {
                                resolved = true;
                            }
                        }
                    }
                    if (resolved == false){
                        doSymbol(arg2, 'U', 0);
                        Globals++;
                    }                 
                }
                if (!isNumber(arg2) && arg2[0] != '\0') symbolics++;
            }
        }
        line_num++;
    }
    rewind(inFilePtr);
    fprintf(outFilePtr, "%i %i %i %i\n", instructions, fills, num_symbols, symbolics);

    /*Second run through to translate the assembly code*/
    line_num = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2)){
    	unsigned int bits = 0;
    	int offsetField = 0;


    	if (!strcmp(opcode, "add")) {
    		bits = 0b000 << 22;
    		unsigned int regA = atoi(arg0) << 19;
    		unsigned int regB = atoi(arg1) << 16;
    		unsigned int dest = atoi(arg2);
    		bits |= regA;
    		bits |= regB;
    		bits |= dest;
    		fprintf(outFilePtr, "%d \n", bits);
    	}
    	else if (!strcmp(opcode, "nor")) {
    		bits = 0b001 << 22;
    		unsigned int regA = atoi(arg0) << 19;
    		unsigned int regB = atoi(arg1) << 16;
    		unsigned int dest = atoi(arg2);
    		bits |= regA;
    		bits |= regB;
    		bits |= dest;
    		fprintf(outFilePtr, "%d \n", bits);
    	}
    	else if (!strcmp(opcode, "lw")) {
    		bits = 0b010 << 22;
    		unsigned int regA = atoi(arg0) << 19;
    		unsigned int regB = atoi(arg1) << 16;
    		bits |= regA;
    		bits |= regB;

    		int mask = 0x0000FFFF;
    		if (!isNumber(arg2)){
    			offsetField = argtoi(arg2, opcode);
                doReloc(arg2, opcode, line_num);
    			bits |= offsetField;
    			if(offsetField < -32768 || offsetField > 32767){
    				exit(1);
    			}
    		}
    		else {
    			if(atoi(arg2) < -32768 || atoi(arg2) > 32767){
    				exit(1);
    			}
    			mask &= atoi(arg2);
    			/*offsetField = atoi(arg2);*/
    			bits |= mask;
    		}
    		/*bits |= offsetField;*/
    		fprintf(outFilePtr, "%d \n", bits);

    	}
    	else if (!strcmp(opcode, "sw")) {
    		bits = 0b011 << 22;
    		unsigned int regA = atoi(arg0) << 19;
    		unsigned int regB = atoi(arg1) << 16;
    		bits |= regA;
    		bits |= regB;

    		int mask = 0x0000FFFF;
    		if (!isNumber(arg2)){
    			offsetField = argtoi(arg2, opcode);
                doReloc(arg2, opcode, line_num);
    			bits |= offsetField;
    			if(offsetField < -32768 || offsetField > 32767){
    				exit(1);
    			}
    		}
    		else {
    			if(atoi(arg2) < -32768 || atoi(arg2) > 32767){
    				exit(1);
    			}
    			mask &= atoi(arg2);
    			/*offsetField = atoi(arg2);*/
    			bits |= mask;
    		}
    		/*bits |= offsetField;*/
    		fprintf(outFilePtr, "%d \n", bits);
    	}
    	else if (!strcmp(opcode, "beq")) {
    		bits = 0b100 << 22;
    		unsigned int regA = atoi(arg0) << 19;
    		unsigned int regB = atoi(arg1) << 16;
    		bits |= regA;
    		bits |= regB;

    		int mask = 0x0000FFFF;
    		if (!isNumber(arg2)){
    			offsetField = argtoi(arg2, opcode) - line_num - 1;
    			mask &= offsetField;
    			bits |= mask;
    			if(offsetField < -32768 || offsetField > 32767){
    				exit(1);
    			}
    		}
    		else {
    			if(atoi(arg2) < -32768 || atoi(arg2) > 32767){
    				exit(1);
    			}
    			mask &= atoi(arg2);
    			/*offsetField = atoi(arg2);*/
    			bits |= mask;
    		}
    		/*bits |= offsetField;*/
    		fprintf(outFilePtr, "%d \n", bits);
    	}
    	else if (!strcmp(opcode, "jalr")) {
    		bits = 0b101 << 22;
    		unsigned int regA = atoi(arg0) << 19;
    		unsigned int regB = atoi(arg1) << 16;
    		bits |= regA;
    		bits |= regB;
    		fprintf(outFilePtr, "%d \n", bits);
    	}
    	else if (!strcmp(opcode, "halt")) {
    		bits = 0b110 << 22;
    		fprintf(outFilePtr, "%d \n", bits);
    	}
    	else if (!strcmp(opcode, "noop")) {
    		bits = 0b111 << 22;
    		fprintf(outFilePtr, "%d \n", bits);
    	}
    	else if (!strcmp(opcode, ".fill")){
    		if (atoi(arg0) < 0){
    			int out = atoi(arg0);
    			fprintf(outFilePtr, "%d \n", out);
    		}
    		else{
    			int out = argtoi(arg0, opcode);
                if (!isNumber(arg0)){
                    doReloc(arg0, opcode, (line_num - instructions));
                }
    			fprintf(outFilePtr, "%d \n", out);
    		}
    	}
    	else{
    		exit(1);
    	}
    	line_num++;
    }
    char *A;
    char B;
    int C;
    for (int i = 0; i < num_symbols; i++){
        A = symbols[i].name;
        B = symbols[i].type;
        C = symbols[i].os;
        fprintf(outFilePtr, "%s %c %i\n", A, B, C);
    }
    char *D;
    for (int i = 0; i < num_relocs; i++){
        A = relocs[i].name;
        D = relocs[i].type;
        C = relocs[i].os;
        fprintf(outFilePtr, "%i %s %s\n", C, D, A);
    }

    return(0);
}

/*
 **********This following function was given to me as starter code for this project*********
 *
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
        char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
        /* reached end of file */
        return(0);
    }

    /* check for line too long (by looking for a \n) */
    if (strchr(line, '\n') == NULL) {
        /* line too long */
        printf("error: line too long\n");
        exit(1);
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label)) {
        /* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);
    return(1);
}

int
isNumber(char *string)
{
    /* return 1 if string is a number */
    int i;
    return( (sscanf(string, "%d", &i)) == 1);
}

int argtoi(char *arg, char *opcode){
	if (isNumber(arg)){
		return atoi(arg);
	}
	else {
		for (int i = 0; i < num_labels; i++){
			if (strcmp(arg, labels[i].name) == 0){
				return labels[i].address;
			}
		}
        if ((arg[0] >= 'A') && (arg[0] <= 'Z') && strcmp(opcode, "beq")){
            return 0;
        }
        if ((arg[0] >= 'A') && (arg[0] <= 'Z') && !strcmp(opcode, "beq")){
            exit(1);
        }
		exit(1);
	}
}

int doSymbol(char *arg, char t, int i){
    if (arg[0] == '\0') return 0;
    for (int j = 0; j < num_symbols; j++){
        if (!strcmp(arg, symbols[j].name)) return 0;
    }
    struct Symbol temp;
    temp.name = (char*)malloc(sizeof(char) * 6);
    strcpy(temp.name, arg);
    temp.type = t;
    temp.os = i;
    symbols[num_symbols] = temp;
    num_symbols++;
    return 0;
}

int doReloc(char *arg, char *t, int i){
    /*printf("1\n");*/
    struct Relocate temp;
    temp.name = (char*)malloc(sizeof(char) * 6);
    strcpy(temp.name, arg);
    temp.type = (char*)malloc(sizeof(char) * 6);
    strcpy(temp.type, t);
    temp.os = i;
    relocs[num_relocs] = temp;
    num_relocs++;
    return 0;
}
