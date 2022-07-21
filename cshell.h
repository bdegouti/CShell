#include<stdio.h>
#include<time.h>
#include<math.h>
#include<stdlib.h>
#include<unistd.h>
#include<assert.h>
#include<float.h>
#include<string.h>
#include<ctype.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>

typedef struct {
    char *name;
   	char *value;
} EnVar;

typedef struct {
char *name;
struct tm time;
int return_value;
} Command;

void logCall();
void theme(char* arg);
void printCall(char** tokens, int numOfArgs);
void exitCall();

void setBlue();
void setYellow();
void setRed();

void freeAll();
void freeTokens(char** tokens, int numOfArgs);
char* stringToLower(char* string);
void addEntryToLog(char* name, int retVal);
void inBuiltCmd(char** tokens, int numOfArgs);
void envar(char** tokens, int numOfArgs);
void checkForEnVarsInCmd(char** tokens, int numOfArgs);
int externalCmd(char** tokens, int numOfArgs);
int checkValidInput(char** tokens, int numOfArgs);
char** parseInput(char* input, int* numOfArgs);
void interactiveMode();
void scriptMode(int argc, char* argv[]);
