#include "cshell.h"

const static int BUFFER_SIZE = 64;
const static int READ_BUFFER = 128;
const static int MAX_NUM_OF_TOKENS = 32;
const static int STARTING_SIZE_OF_LOG = 16;
const static int STARTING_SIZE_OF_ENVARS = 16;
const static int MAX_SIZE_OF_INPUT = 256;
const static char DELIMITER[] = " =\n";
int running, logIndex, logMultiple, varIndex, varMultiple, Running;
Command** pLog;
EnVar** enVars;

int main(int argc, char* argv[]){
    logIndex = 0;
    logMultiple = 0;
    pLog = (Command**) malloc(sizeof(Command*)*STARTING_SIZE_OF_LOG);
    if(pLog == NULL){
        printf("CRITICAL ERROR: failed to allocate base rescources for log");
        exit(0);
    }

    varIndex = 0;
    varMultiple = 0;
    enVars = (EnVar**) malloc(sizeof(Command*)*STARTING_SIZE_OF_ENVARS);
    if(pLog == NULL){
        printf("CRITICAL ERROR: failed to allocate base rescources for Environment Variables");
        exit(0);
    }
    Running = 1;
    if(argc <= 1){
        interactiveMode();
    }
    else{
        scriptMode(argc, argv);
    }
}

//ADDS ENTRY TO LOG, INCREASES LOG MEMORY DYNAMICALLY IF NEEDED
void addEntryToLog(char* name, int retVal){
    if(logIndex >= (STARTING_SIZE_OF_LOG * pow(2, logMultiple))-1){
        logMultiple++;
        Command** tmp = (Command**)realloc(pLog, (STARTING_SIZE_OF_LOG * pow(2, logMultiple)*sizeof(Command*)));
        if(tmp == NULL){
            printf("ERROR: realloc failed at log");
            return;
        }
        pLog = tmp;
    }
    Command* newCmd = malloc(sizeof(Command));
    char* newName = malloc(sizeof(char)*BUFFER_SIZE);
    strcpy(newName, name);
    newCmd->name = newName;
    newCmd->return_value = retVal;
    
    time_t rtime;
    rtime = time(NULL);
    struct tm* tmp = localtime(&rtime);
    newCmd->time = *tmp;
    pLog[logIndex] = newCmd;
    logIndex++;
}

//CONVERTS STRING TO LOWER CASE
char* stringToLower(char* string){
    char* tmp = string;
    for(int i = 0; string[i]; i++){
        string[i] = tolower(string[i]);
    }
    return tmp;
}

//THEME FUNCTIONS (CHANGE COLOUR OF OUT TEXT)

void setRed(){
    printf("\033[1;31m");
}

void setYellow() {
    printf("\033[1;33m");
}

void setBlue() {
    printf("\033[1;34m");
}

//------
void freeAll() {
    for(int i = 0; i < varIndex; i++){
        free(enVars[i]->name);
        free(enVars[i]->value);
        free(enVars[i]);
    }
    free(enVars);
    
    for(int i = 0; i < logIndex; i++){
        free(pLog[i]->name);
        free(pLog[i]);
    }
    free(pLog);
}
//INBUILT COMMANDS
void exitCall(char** tokens, int numOfArgs){    
    printf("Bye!\n");
    Running = 0;
}

void printCall(char** tokens, int numOfArgs){
    if(numOfArgs <= 1){
        addEntryToLog("print", -1);
        return;
    }
    for(int i = 1; i < numOfArgs; i++){
        printf("%s ", tokens[i]);
    }
    printf("\n");
    addEntryToLog("print", 0);
}

void theme(char* arg){
    char* colour = stringToLower(arg);
    if(!strcmp(colour, "red")){
        setRed();
    }
    else if(!strcmp(colour, "yellow")){
        setYellow();
    }
    else if(!strcmp(colour, "blue")){
        setBlue();
    }
    else{
        printf("Unsupported theme: %s\n", colour);
        addEntryToLog("theme", -1);
        return;
    }
    addEntryToLog("theme", 0);
}

void logCall(){
    if(pLog != NULL && logIndex < (STARTING_SIZE_OF_LOG * pow(2, logMultiple))){
        for(int i = 0; i < logIndex; i++){
            char* tm = asctime(&pLog[i]->time);
            printf("%s %s %d\n", tm, pLog[i]->name, pLog[i]->return_value);
        }
        addEntryToLog("log", 0);
        return;
    }
    addEntryToLog("log", -1);
}
//IN BUILT COMMAND CHECKER, CHECKS IMPLEMENTED COMMANDS AND RUNS
void inBuiltCmd(char** tokens, int numOfArgs){
    checkForEnVarsInCmd(tokens, numOfArgs);
    if(!strcmp(tokens[0], "exit")){
        exitCall(tokens, numOfArgs);
        return;
    }
    else if(!strcmp(tokens[0], "print")){
        printCall(tokens, numOfArgs);
        return;
    }
    else if(!strcmp(tokens[0], "theme")){
        theme(tokens[1]);
        return;
    }
    else if(!strcmp(tokens[0], "log")){
        logCall();
        return;
    }
    else{
        //should never reach here!
        printf("ERROR: no inbuilt command of this name\n");
        addEntryToLog("UNKOWN LOCAL COMMAND", -1);
    }
}

void checkForEnVarsInCmd(char** tokens, int numOfArgs){
    for(int i = 1; i < numOfArgs; i++){
        if(tokens[i][0] == '$'){
            for(int k = 0; k < varIndex; k++){
                if(strcmp(tokens[i], enVars[k]->name)==0){
                    char* tmpStr = (char*)malloc(sizeof(char)*BUFFER_SIZE);
                    char* destroy;
                    strcpy(tmpStr, enVars[k]->value);
                    destroy = tokens[i];
                    tokens[i] = tmpStr;
                    free(destroy);
                    return;
                }
            }
        }
    }
}

void envar(char** tokens, int numOfArgs){
    if(varIndex >= (STARTING_SIZE_OF_ENVARS * pow(2, varMultiple))-1){
        varMultiple++;
        enVars = (EnVar**)realloc(pLog, (STARTING_SIZE_OF_ENVARS * pow(2, varMultiple)));
    }
    if(numOfArgs == 2){
        for(int k = 0; k < varIndex; k++){
            if(tokens[1] == enVars[k]->name){
                enVars[k]->value = tokens[2];
                return;
            }
        }
        EnVar* newEV = (EnVar*) malloc(sizeof(EnVar));
        if(newEV == NULL){
            printf("ERROR: malloc failure at Environment Variable\n");
            addEntryToLog("N/A",-1);
            return;
        }

        char* tmpStrName = (char*) malloc(sizeof(char)*BUFFER_SIZE);
        if(tmpStrName == NULL){
            printf("ERROR: malloc failure at EnVar->name\n");
            addEntryToLog("N/A",-1);
            return;
        }

        char* tmpStrValue = (char*) malloc(sizeof(char)*BUFFER_SIZE);
        if(tmpStrValue == NULL){
            printf("ERROR: malloc failure at EnVar->name\n");
            addEntryToLog("N/A",-1);
            return;
        }

        strcpy(tmpStrName, tokens[0]);
        strcpy(tmpStrValue, tokens[1]);
        newEV->name = tmpStrName;
        newEV->value = tmpStrValue;

        enVars[varIndex] = newEV;
        varIndex++;

        char* entry = (char*) malloc(sizeof(char)*((2*BUFFER_SIZE)+1));
        sprintf(entry, "%s=%s", newEV->name, newEV->value);
        addEntryToLog(entry,0);
        free(entry);
    }
    else{
        printf("ERROR: invalid number of arguments!\n");
    }
}

int externalCmd(char** tokens, int numOfArgs){
    char buffer[READ_BUFFER];
    char** args = (char**)malloc(sizeof(char*)*numOfArgs);
    int ppd[2];
    int fc;

    for(int i = 0; i < numOfArgs; i++){
        args[i] = tokens[i];
    }
    memset(buffer, 0, READ_BUFFER);

    if(pipe(ppd) < 0){
        perror("pipe failed\n");
        addEntryToLog(tokens[0], -1);
        return 0;
    }
    if((fc = fork()) < 0){
        perror("fork failed\n");
        addEntryToLog(tokens[0], -1);
        return 0;
    }

    if(fc == 0){
        close(ppd[0]);
        dup2(ppd[1], STDOUT_FILENO);
        dup2(STDOUT_FILENO, STDERR_FILENO);
        close(ppd[1]);

        char loc[20];
        strcpy(loc, "/usr/bin/");
        strcat(loc, tokens[0]);
        if(execv(loc, args) == -1){
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
        return 1;
    }
    else{
        int status;
        close(ppd[1]);
        wait(&status);

        if(WIFEXITED(status)!=0){
            if (WEXITSTATUS(status) == 1){
                return 0;
            }
        }
        else{
            return 0;
        }
        
        while(read(ppd[0], buffer, READ_BUFFER)){
            printf("%s", buffer);
        }

        free(args);
        
        addEntryToLog(tokens[0], 0);
        return 1;
    }
}

//CHECKS IF INPUT IS VALID, RUNS IF IT IS.
int checkValidInput(char** tokens, int numOfArgs){
    if(!strcmp(tokens[0], "exit") || !strcmp(tokens[0], "print") || !strcmp(tokens[0], "log") || !strcmp(tokens[0], "theme")){
        inBuiltCmd(tokens, numOfArgs);
        return 1;
    }
    else if(tokens[0][0] == '$'){
        envar(tokens, numOfArgs);
        return 1;
    }
    else{
        if(externalCmd(tokens, numOfArgs)){
            return 1;
        }
        return 0;
    }
    
}

//PARSES ENTERED COMMAND INTO TOKENS
char** parseInput(char* input, int* numOfArgs){
    char** tokens = (char**)malloc(sizeof(char*)*MAX_NUM_OF_TOKENS);
    int i = 0;
    char* token;
    token = strtok(input, DELIMITER);

    
    while(token != NULL && i < MAX_NUM_OF_TOKENS){
        char* tmp = (char*)malloc(sizeof(char)*BUFFER_SIZE);
        strcpy(tmp, token);
        tokens[i] = tmp;
        token = strtok(NULL, DELIMITER);
        i++;
    }
    *numOfArgs = i;
    return tokens;
}

void freeTokens(char** tokens, int numOfArgs){
    int i;
    for(i = 0; i < numOfArgs; i++){
        free(tokens[i]);
    }
    free(tokens);
}

//INTERACTIVE MODE, IMPLEMENTS LOOP FOR REPEATED COMMAND ENTRY THROUGH CONSOLE
void interactiveMode(){
    int numOfArgs;
    char input[MAX_SIZE_OF_INPUT];
    while(Running){
        printf("cshell$ ");
        fgets(input, MAX_SIZE_OF_INPUT, stdin);
        char** tokens = parseInput(input, &numOfArgs);
        if(numOfArgs>0){
            if(!checkValidInput(tokens, numOfArgs)){
                printf("Missing keyword or command, or permission problem\n");
            }
        }
        freeTokens(tokens, numOfArgs);
    }
    freeAll();
}

//TODO: add script reading mode
void scriptMode(int argc, char* argv[]){
    if(argc>2){
        printf("ERROR: to many args!\n");
        exit(0);
    }
    FILE* file;
    char* line = (char*)malloc(sizeof(char)*MAX_SIZE_OF_INPUT);
    size_t maxSize = MAX_SIZE_OF_INPUT;
    int numOfArgs;
    file = fopen(argv[1], "r");
    if(file == NULL){
        printf("Unable to read script file: %s\n", argv[1]);
        exit(0);
    }
    while(getline(&line, &maxSize, file) != -1 && Running){
        char** tokens = parseInput(line, &numOfArgs);
        if(numOfArgs>0){
            if(!checkValidInput(tokens, numOfArgs)){
                printf("Missing keyword or command, or permission problem\n");
            }
        }
        freeTokens(tokens, numOfArgs);
    }
    freeAll();
    free(line);
    fclose(file);
}