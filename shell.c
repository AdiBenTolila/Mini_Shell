/**
     The program receives a commands(she can get more than one command seperated by "|") from the user and the command will be sent to the operating system for execution,
     using the fork commands (which create the son process) and execvp (which executes the command)
     and pipe(indicates to the shell to route the STDOUT of the command before it to the STDIN of the command following it)
     The run will be by the program creating a son process and the pipes.
     The son will actually run the command using the execvp command and by routing the output and input
     The program also supports the char '&'-If the character appears at the end of the command, it will put the command to run in the background
     and allow the user to enter additional commands
     in addition support the command "nohup" in the beginning of the string-the command will allow the program to run even after the SHELL is closed (even if the user entered the word done)
     User instructions and program process as follows:
   - the numbers of the pipe and the number of command that the user enter(including the word cd but not done),
     will be printed on the screen when the user end the program(enter the word done)
   - The command will be inserted into the file- "file.txt" (just if its valid command and not cd command and not a commands that have more than 2 pipes)
   - When the user wants to see the history of the string that in the file he can see them by inserting the word: "history"
     (the word history can appear with pipe)
     And if the user wants to end the program, he will enter the word: "done"
   - when the user want to use one of the commands in the history file he need to enters in the start exclamation mark and then the line of the command in the history(can appear with pipe)

   - Input Discounts:
     invalid input-if in the start or in the end of the string there is a space or the string is just enter
     The maximum string length is 512 characters.
     There is no limit to the number of words (as long as the string is in the limit length).
     There is no limit to the number of characters of a word (as long as the string is in the limit length).
     string cant start or end with pipe
     There can be several spaces between one word and another, but it can be assumed that there is no other character between them (like '\ r', '\ t').
     the string can't be with more than 2 pipes
     if we have invalid command but in the last command there is an valid command the command will be executed and her output will be printed in addition to error message
     as in the original shell




     @author - Adi Ten Tolila
**/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>

/**
     const variables
**/
#define SIZE 512
#define PATH "file.txt"
#define NOHUP_PATH "nohup.txt"
#define IN_HISTORY (-2)
#define TRUE 1
#define FALSE 0


/**
    methods
**/
void makePath();//A method that gets the name of the current folder
void numOfWord(const char*,int*);//method that count how many words there is to allocate and add to the total.
void doCommandHistory(char[],const int*,int*,int*,int,const char[]);//A method that put the new command from the history in the inputCommand
void checkFromHistory(char*,int*);//A method that check if there is exclamation mark and put the new command from the history in the right place
void stringAnalysis(const char*,char**,int*,int*,int*,int*,int*,int[],int[]);//method that reserved a command and put the command in the array command
void insert(char**,char*,const int*,int*);//method that insert word from the method stringAnalysis and creat the array of all the words in the command.
void writToFile(char*);//method that write to the file the sentence that reserved
void readFromFile();//method that open the file and print all the data inside - when the user insert the word "history".

void executeOneCommand(char**,int,int[],int[]);//method that creates the new process.
void execute(char**, int,int,int,int,int[],int[]);//method that creates all the new process with the pipe.
void execvpCommand(char**,int, int,int[],const int[],int);//method that do the execvp and check before if the command is history or have nohup
void freeAllocate(char**,int);//method that frees all the allotted space.
void handler(int);//method to the signal that work when the status of one of the sons change and wait
void nohupMethod(char**,int);//method that work when the command is nohup command and will allow the program to run even after the SHELL is closed, output will be appended to nohup.txt file and input channel will be closed

//the following function  indicates to the shell to route the STDOUT of the command before it to the STDIN of the command following it:
void son1pipe1(char**,int[],int,int[],int[]);
void son2pipe1(char**,int[],int,int,int[],int[]);
void son1pipe2(char**,int[],int[],int,int[],int[]);
void son2pipe2(char**,int[],int[],int,int,int[],int[]);
void son3pipe2(char**,int[],int[],int,int,int,int[],int[]);


/**
    main program
**/
int main() {
    signal(SIGCHLD,handler);
    char inputCommand[SIZE + 2];//for the sentence from the user
    int totalCommand=0;
    int totalWordsCommand=0;
    int totalPipes=0;
    while(1){//the loop end when the user press exit
        makePath();
        fgets(inputCommand, SIZE + 2, stdin);  //Receives a sentence from the user
        inputCommand[strlen(inputCommand) - 1]='\0';
        int countChars = 0;
        int countWords = 0;
        int isCd = FALSE;//if there is a command with cd
        int notInHistory = FALSE;
        int isAmpersand[]={FALSE,FALSE,FALSE};//array to know if there is in one of the commands ampersand
        int isNohupArray[]={FALSE,FALSE,FALSE};//array to know if there is in one of the commands "nohup"

        //if in the start or in the end of the string there is a space or the string is just enter - invalid input
        if(inputCommand[0]==' '||inputCommand[(strlen(inputCommand))-1]==' '||strcmp(inputCommand,"")==0) {
            fprintf(stderr,"invalid command\n");
            continue;
        }

        //if there is exclamation mark the method put the new command from the history in the right place
        checkFromHistory(inputCommand,&notInHistory);
        if(notInHistory == TRUE){//the number of command that the user want not exist
            printf("NOT IN HISTORY\n");
            totalCommand++;
            continue;
        }

        int numOfPipes=0,secondCommand=0, thirdCommand=0;
        numOfWord(inputCommand, &countWords);//count number of words and pipe to allocate
        char** commandArray = (char**)malloc(sizeof(char*) * (countWords+1));
        if(commandArray == NULL){
            perror("ERR");
            exit(1);
        }

        countWords=0;
        countChars=0;

        stringAnalysis(inputCommand, commandArray, &countWords, &countChars, &numOfPipes, &secondCommand, &thirdCommand,isAmpersand,isNohupArray); /*get the number of words and pipes in the sentence
                                                                                                    and also put in command array word by word from
                                                                                                    inputCommand(the command the user enter)
                                                                                                    except the word "nohup" or &
                                                                                                    also the method fills the isAmpersand array and the isNohupArray in accordance*/

        if(commandArray[0] ==NULL || commandArray[secondCommand] ==NULL || commandArray[thirdCommand] ==NULL){
            freeAllocate(commandArray, countWords);
            fprintf(stderr,"invalid command\n");
            continue;
        }

        if(strcmp(commandArray[0], "cd") == 0 || strcmp(commandArray[secondCommand], "cd") == 0 || strcmp(commandArray[thirdCommand], "cd") == 0){//one of the command is cd - not supported - continue the loop
            isCd = TRUE;
        }
        if(countChars!=0){
            totalPipes+=numOfPipes;
            if(isCd == FALSE && numOfPipes<3 ){//If the command is invalid or not a cd or with more than 2 pipes(can take from the history and the string can be with more than 2 pipe)we will put it in the history file
                writToFile(inputCommand);
            }
            if(strcmp(inputCommand,"done") != 0){//if it's not the command done
                totalCommand++;
            }
        }

        if(strcmp(inputCommand, "done") == 0){//the command is done - end the program
            printf("Number of commands: %d\n",totalCommand);
            printf("Number of pipes: %d\n",totalPipes);
            printf("See you Next time !\n");
            freeAllocate(commandArray, countWords);
            break;
        }
        if(isCd == TRUE){//the command is cd - not supported - continue the loop
            fprintf(stderr,"command not supported (yet)\n");
            freeAllocate(commandArray, countWords);
            continue;
        }

        execute(commandArray, countWords , numOfPipes, secondCommand, thirdCommand,isAmpersand ,isNohupArray);
        freeAllocate(commandArray,countWords);

    }
    return 0;
}

/**
 * A method that gets the name of the current folder
 **/
void makePath(){
    char cwd[PATH_MAX];//current dir
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s>", cwd);
    } else {
        perror("getcwd() error");
        exit(0);
    }
}

/**
 *method that count how many words and pipes there is to allocate and add to the total.
 *  @param inputCommand-The command from the user
    @param countWords-count how many words there is
**/
void numOfWord(const char* inputCommand, int* countWords){
    int countChars = 0;
    int countPipe=0;
    for(int i = 0; inputCommand[i] != '\0' ; i++){//loop-char by char
        if(inputCommand[i]=='|')//count the pipes
            (countPipe)++;

        if(((inputCommand[i] == ' ' && ((inputCommand[i + 1] != ' ') && inputCommand[i + 1] != '\0')) && (countChars) != 0)||inputCommand[i] == '|'&& inputCommand[i + 1] != ' '&&inputCommand[i - 1] != ' '){//counting the words according the space in the String

            (*countWords)++;
    }
        else if(inputCommand[i] != ' ' && inputCommand[i] != '\0' && inputCommand[i] != '|')//if it's not space its char
            (countChars)++;

        else
            continue;
    }
    (*countWords)=(*countWords)+(countPipe)+1;
}

/**
 * A method that put the new command from the history in the newInputCommand
 *  @param insertString-the new string that combination of the commands from the original string and history
    @param option-check if the number exists in the history
    @param wordFromHistory - the line number
    @param charInsert - counter of the insertString
    @param fromPlace - to know when the exclamation mark
    @param temp - that the new string will stay the same with all the spaces
 **/
void doCommandHistory(char insertString[], const int* wordFromHistory, int* option, int* charInsert,int fromPlace,const char temp[]){
    char fromFile[SIZE + 2];
    fromFile[0]='\0';
    int count=1;
    FILE *fp;
    fp = fopen(PATH,"r");//open the file for reading
    if(fp == NULL)
    {
        return;
    }

    while(!feof(fp))//pass all the sentence in the file
    {

        fromFile[0]='\0';//before insert a new sentence we erase the current sentence.
        fgets(fromFile,SIZE+2,fp);
        if(strcmp(fromFile,"\0")!=0) {//check if the sentence doesn't change,if it's not change its mean we are not in the end of the file
            // ,and like that the last sentence in the file will not duplicate(because the last line is empty)
            fromFile[strlen(fromFile) - 1]='\0';
            if ((count) == (*wordFromHistory)) {//we got to the request line
                if ((*charInsert) < SIZE) {
                    for (int i = 0; fromFile[i] != '\0'; ++i) {
                        insertString[(*charInsert)++] = fromFile[i];//combine the command from the user and the command from the file
                    }
                    *option = -1;//if option change to -1 we found the line in the history file
                    break;
                }
            }
        }
        count++;
    }
    if(*option==-1)
        *option=IN_HISTORY;

    fclose(fp);//close the file
}

/**
 * A method that check if there is exclamation mark and put the new command from the history in the right place
 *  @param inputCommand-The command from the user
    @param notInHistory - if the number not in the history
 **/
void checkFromHistory(char* inputCommand,int* notInHistory){
    char insertString[SIZE + 2];//to change the original string(from the history)
    char temp[SIZE + 2];//temporary string word by word(mostly to know if there is exclamation mark)
    insertString[0] = '\0';//initialize
    temp[0] = '\0';//initialize

    int countCharsForWords = 0;
    int numberForHistory = 0;//to know the line number that we want from the history
    int charInsert=0;//counter for the insertSting array
    int countTemp = 0;//counter for the temp array
    int fromPlace=0;//when the method combine we want also the spaces,so that how we know how many spaces we need before the string from the history
    int hasExclamationMark = FALSE;
    for (int i = 0; inputCommand[i] != '\0'; ++i) {
        if(inputCommand[i] == '!' && (inputCommand[i+1] >= 48 && inputCommand[i+1] <= 57)){//find the exclamation mark to switch
            for (int j = 0; j < countTemp; ++j)
                insertString[(charInsert)++] = temp[j];//combine the command from the user and the command from the file

            int numberFromHistory=0;
            int option=0;
            int j=i+1;
            while (inputCommand[j] >= 48 && inputCommand[j] <= 57) {
                numberFromHistory*=10;
                numberFromHistory+= inputCommand[j] - 48;
                j++;
            }

            if(numberFromHistory >= 0)
                doCommandHistory(insertString,&numberFromHistory,&option,&charInsert,fromPlace,temp);
            if(option != IN_HISTORY && numberFromHistory >= 0 )//to know if the line number exist in the history
                *notInHistory = TRUE;

            i = j-1;
            countTemp=0;
            temp[0] = '\0';//initialize to add another word

        }
        else{
            temp[countTemp++] = inputCommand[i];//put the new word in the temporary string
        }
    }
    for (int j = 0; j < countTemp; ++j)
        insertString[(charInsert)++] = temp[j];//combine the command from the user and the command from the file

    insertString[(charInsert)]='\0';
    strncpy(inputCommand,insertString,SIZE);//copy the new string to the original string

}

/**
 *method that reserved a command and analyse how many word and pipes there is in the command and also put the command in the array command and also fills the isAmpersand array and the isNohupArray in accordance
 @param inputCommand-The command from the user
 @param commandArray-command array
 @param countWords-count how many words there is
 @param countChars-count how many chars there is
 @param numOfPipes-how many pipe there is
 @param secondCommand - to know where is the second command if there is
 @param thirdCommand - to know where is the third command if there is
 @param isAmpersand-to know which command have ampersand
 @param isNohupArray-to know which command have nohup
**/
void stringAnalysis(const char* inputCommand, char** commandArray, int* countWords, int* countChars ,int* numOfPipes,int* secondCommand,int* thirdCommand,int isAmpersand[],int isNohupArray[]){
    char insertWord[SIZE + 2];//to choose options
    insertWord[0]='\0';
    int countCharsForWords=0;
    int isInsert = FALSE;

    int isQuotationMark = FALSE;
    int ampersand = FALSE;

    int amperPlace=0;
    int nohupPlace=0;
    for(int i = 0; inputCommand[i] != '\0' ; i++){//loop-char by char
        if(isQuotationMark == FALSE && ((((inputCommand[i] == ' ' && (inputCommand[i + 1] != ' ') && inputCommand[i + 1] != '\0' && inputCommand[i + 1] != '|' && (inputCommand[i - 1] != '|' && (inputCommand[i - 1] != ' '))) && (*countChars) != 0)&&isInsert == FALSE)||
                                        (inputCommand[i] == ' ' && (inputCommand[i + 1] != ' ')&& (inputCommand[i - 1] == ' ') &&isInsert == FALSE &&inputCommand[i + 1] != '|') )){//counting the words according the space in the String


            if(insertWord[0]=='n' && insertWord[1]=='o'&& insertWord[2]=='h'&& insertWord[3]=='u'&& insertWord[4]=='p' && countCharsForWords==5 && (*countChars<6||commandArray[(*countWords)-1] == NULL)){
                isNohupArray[nohupPlace] = TRUE;
            }
            else{
                insert(commandArray, insertWord, &countCharsForWords , countWords);//insert word by word to the command array
            }

            insertWord[0]='\0';
            countCharsForWords=0;
            isInsert=TRUE;
        }
        else if(isQuotationMark == TRUE && (inputCommand[i] == ' ' ||inputCommand[i] == '|')){
            insertWord[countCharsForWords++]=inputCommand[i];//add the chars and creat the words
        }
        else if(inputCommand[i] != ' ' && inputCommand[i] != '\0' && inputCommand[i] != '|' ){//if it's not space its char
            (*countChars)++;
            insertWord[countCharsForWords++]=inputCommand[i];//add the chars and creat the words
            isInsert=FALSE;
            if(inputCommand[i] == '\"'){
                isQuotationMark = !isQuotationMark;
            }
        }
        else if(inputCommand[i] == '|' && isInsert == FALSE && isQuotationMark!= TRUE){
            (*countChars)++;
            isInsert=TRUE;

            if(insertWord[countCharsForWords-1]=='&'){//check if there is ampersand
                isAmpersand[amperPlace] = TRUE;
                ampersand=TRUE;
                countCharsForWords--;
                insert(commandArray, insertWord, &(countCharsForWords) , countWords);//insert word by word to the command array without &


            }
            else{
                insert(commandArray, insertWord, &countCharsForWords , countWords);//insert word by word to the command array
            }
            insertWord[0]='\0';
            countCharsForWords=0;
            amperPlace++;
            nohupPlace++;
            ampersand = FALSE;
            commandArray[(*countWords)++]=NULL;//after every command there is null
            (*numOfPipes)++;
            if((*numOfPipes)==1)
                (*secondCommand) = (*countWords);//to know where is the second command start
            else if((*numOfPipes)==2)
                (*thirdCommand)= (*countWords);//to know where is the third command start
        }
        else{
            continue;
        }

    }
    if(insertWord[countCharsForWords-1]=='&'){//check if there is ampersand
        isAmpersand[amperPlace] = TRUE;
        ampersand=TRUE;
        countCharsForWords--;
        insert(commandArray, insertWord, &countCharsForWords , countWords);//insert word by word to the command array
    }
    else{
        insert(commandArray, insertWord, &countCharsForWords , countWords);//insert word by word to the command array
    }
    commandArray[(*countWords)]=NULL;
}

/**
 *method that insert word from the method stringAnalysis and creat the array of all the words in the command.
    @param commandArray-command array
    @param insertWord-The word that we want to insert
    @param countCharsForWords-how many chars there is in the word that we want to insert
    @param countWords-count how many words there is
**/
void insert(char** commandArray, char* insertWord, const int* countCharsForWords , int* countWords){
    insertWord[(*countCharsForWords)]='\0';
    if(strcmp(insertWord,"")!=0){
        char* word = (char*)malloc(sizeof(char)*((*countCharsForWords)+1));
        if(word == NULL){
            perror("ERR");
            exit(1);
        }
        strcpy(word, insertWord);
        commandArray[(*countWords)++]=word;//insert the new word to the commands d array
    }

}
/**
 *method that write to the file the sentence that reserved
 @param inputCommand-The command from the user
**/
void writToFile(char* inputCommand){
    FILE *fp;
    fp = fopen(PATH,"a"); //open the file for writ(a - save the data in the file and does not erase what is inside

    if(fp == NULL)
    {
        fprintf(stderr,"cannot open file\n");
        exit(1);
    }

    fprintf(fp, "%s\n", inputCommand);//copy the word to the file
    fclose(fp); //close the file history
}

/**
 *method that open the file and print all the data inside - when the user insert the word "history".
**/
void readFromFile(){
    int count=1;
    FILE *fp;
    fp = fopen(PATH,"r");//open the file for reading
    if(fp == NULL)
    {
        exit(1);
    }
    char fromFile[SIZE+2];
    while(!feof(fp))//pass all the sentence in the file
    {
        strcpy(fromFile,"\0");//before insert a new sentence we erase the current sentence.
        fgets(fromFile,SIZE+2,fp);
        if(strcmp(fromFile,"\0")!=0)//check if the sentence doesn't change,if it's not change its mean we are not in the end of the file
            // ,and like that the last sentence in the file will not duplicate(because the last line is empty)

            printf("%d: %s",count++,fromFile);//write the word to the file
    }
    fclose(fp);//close the file
}

/**
 *method that creates all the new process with the pipe.
   @param commandArray-command array
   @param countWords-count how many words there is
   @param numOfPipes - how many pipe there is and do the commands from the array
   @param secondCommand - the place of the second command
   @param thirdCommand - the place of the third command
   @param isAmpersand - to know if let the command to run in the background and allow the user to enter more commands
   @param isNohup - to know if Allow the program to continue running even after the shell closes
**/
void execute(char** commandArray, int countWords ,int numOfPipes,int secondCommand,int thirdCommand,int isAmpersand[], int isNohup[]) {
    int pidSon[]={0,0,0};
    if (numOfPipes == 0) {
        executeOneCommand(commandArray,countWords, isAmpersand, isNohup);
    }
    else if (numOfPipes == 1) {//2 commands
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("cannot open pipe");
            freeAllocate(commandArray, countWords);
            exit(EXIT_FAILURE);
        }
        pid_t pid = fork();
        if (pid == -1) {
            perror("ERR");
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            freeAllocate(commandArray, countWords);
            exit(1);
        } else if (pid == 0) { // in the child process
            son1pipe1(commandArray,pipe_fd,countWords, isAmpersand, isNohup);
        }
        pidSon[0]=pid;
        // in the parent process
        pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("ERR");
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            freeAllocate(commandArray, countWords);
            exit(1);

        } else if (pid2 == 0) {//in the second son
            son2pipe1(commandArray,pipe_fd,countWords,secondCommand, isAmpersand, isNohup);
        }
        // in the parent process
        pidSon[1]=pid2;
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        for (int i = 0; i <2 ; ++i) {
            if(isAmpersand[i] == FALSE)
                waitpid(pidSon[i],NULL,0);
        }
        fflush(stdout); //print everything in the stdout
        fflush(stderr);

    }
    else if(numOfPipes == 2){//3 commands
        int pipe_fd[2];
        int pipe_fd2[2];

        if (pipe(pipe_fd) == -1) {
            perror("cannot open pipe");
            freeAllocate(commandArray, countWords);
            exit(EXIT_FAILURE);
        }
        if (pipe(pipe_fd2) == -1) {
            perror("pipe");
            freeAllocate(commandArray, countWords);
            exit(1);
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("ERR");
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            close(pipe_fd2[0]);
            close(pipe_fd2[1]);
            freeAllocate(commandArray, countWords);
            exit(1);
        } else if (pid == 0) { // in the child process
            son1pipe2(commandArray,pipe_fd,pipe_fd2,countWords, isAmpersand, isNohup);
        }
        else {// in the parent process
            pidSon[0]=pid;
            pid_t pid2 = fork();
            if (pid2 == -1) {
                perror("ERR");
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                close(pipe_fd2[0]);
                close(pipe_fd2[1]);
                freeAllocate(commandArray, countWords);
                exit(1);
            } else if (pid2 == 0) {//in the second son
                son2pipe2(commandArray,pipe_fd,pipe_fd2,countWords,secondCommand, isAmpersand, isNohup);
            }else{
                // in the parent process
                pidSon[1]=pid2;
                pid_t pid3 = fork();
                if (pid3 == -1) {
                    perror("ERR");
                    close(pipe_fd[0]);
                    close(pipe_fd[1]);
                    close(pipe_fd2[0]);
                    close(pipe_fd2[1]);
                    freeAllocate(commandArray, countWords);
                    exit(1);
                } else if (pid3 == 0) {//in the third son
                    son3pipe2(commandArray,pipe_fd,pipe_fd2,countWords,secondCommand,thirdCommand, isAmpersand, isNohup);
                }
                // in the parent process
                pidSon[2]=pid3;
            }

            close(pipe_fd[0]);
            close(pipe_fd[1]);
            close(pipe_fd2[0]);
            close(pipe_fd2[1]);

            for (int i = 0; i <3 ; ++i) {//wait to the sun accordingly the ampersand
                if(isAmpersand[i] == FALSE )
                    waitpid(pidSon[i],NULL,0);
            }
            fflush(stdout); // Will now print everything in the stdout buffer
            fflush(stderr); // Will now print everything in the stdout buffer
        }
    }
    else{
        fprintf(stderr,"not support more then two pipes\n");
    }
}
/**
 *method that creates the new process of One command.
   @param commandArray-command array
   @param countWords-count how many words there is
   @param isAmpersand - to know if let the command to run in the background and allow the user to enter more commands
   @param isNohup - to know if Allow the program to continue running even after the shell closes

**/
void executeOneCommand(char** commandArray, int countWords, int isAmpersand[], int isNohup[]){
    pid_t pid = fork();
    if(pid == -1){
        perror("ERR");
        freeAllocate(commandArray, countWords);
        exit(1);
    }
    else if(pid==0){//son process
        execvpCommand(commandArray,countWords,0, isAmpersand, isNohup,0);
    }
    else{
        if(isAmpersand[0] == FALSE){
            waitpid(pid,NULL,0);
            fflush(stdout); //print everything in the stdout
            fflush(stderr);

        }
    }
}
/**
 *method that do the execvp and check before if the command is history or have nohup
   @param commandArray-command array
   @param countWords-count how many words there is
   @param startCommand - to know where is the command is start
   @param isAmpersand - to know if let the command to run in the background and allow the user to enter more commands
   @param isNohup - to know if Allow the program to continue running even after the shell closes
   @param commandPlace - to know which command we are

**/
void execvpCommand(char** commandArray,int countWords, int startCommand, int isAmpersand[], const int isNohup[],int commandPlace){
    if(isNohup[commandPlace] == TRUE){//if its nohup
        nohupMethod(commandArray, countWords);
    }
    if(strcmp(commandArray[startCommand],"history")==0 && commandArray[startCommand+1]==NULL){//the command is history - all the data in the file will print
        freeAllocate(commandArray, countWords);
        readFromFile();
        exit(0);
    }
    else{
        execvp(commandArray[startCommand], commandArray+startCommand);
        perror("execvp");//if the command didn't work
        freeAllocate(commandArray, countWords);
        exit(0);
    }
}

/**
 *sons methods that indicates to the shell to route the STDOUT of the command before it to the STDIN of the command following it
   @param commandArray-command array
   @param pipe_fd - pipe array
   @param pipe_fd2- pipe array 2
   @param countWords-count how many words there is
   @param isAmpersand - to know if let the command to run in the background and allow the user to enter more commands
   @param isNohup - to know if Allow the program to continue running even after the shell closes
**/
void son1pipe1(char** commandArray,int pipe_fd[],int countWords, int isAmpersand[], int isNohup[]){
    int value = dup2(pipe_fd[1],STDOUT_FILENO); //duplicate the output to the pipe (send stdout output to the pipe)
    if (value == -1) {
        fprintf(stderr, "dup2 failed");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        perror("execvp");//if the command didn't work
        freeAllocate(commandArray, countWords);
        exit(1);
    }
    //close pipe
    close(pipe_fd[1]);
    close(pipe_fd[0]);
    execvpCommand(commandArray,countWords,0, isAmpersand, isNohup,0);
}
void son2pipe1(char** commandArray,int pipe_fd[],int countWords,int secondCommand, int isAmpersand[], int isNohup[]){
    int value = dup2(pipe_fd[0],STDIN_FILENO); //duplicate the pipe to stdin (take input to stdin)
    if (value == -1) {
        fprintf(stderr, "dup2 failed");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        perror("execvp");//if the command didn't work
        freeAllocate(commandArray, countWords);
        exit(1);
    }
    //close pipe
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    execvpCommand(commandArray,countWords,secondCommand, isAmpersand, isNohup,1);
}
void son1pipe2(char** commandArray,int pipe_fd[],int pipe_fd2[],int countWords, int isAmpersand[], int isNohup[]){
    int value = dup2(pipe_fd[1],STDOUT_FILENO); //duplicate the output to the pipe (send stdout output to the pipe)
    if (value == -1) {
        fprintf(stderr, "dup2 failed");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        close(pipe_fd2[0]);
        close(pipe_fd2[1]);
        perror("execvp");//if the command didn't work
        freeAllocate(commandArray, countWords);
        exit(1);
    }
    //close pipes
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    close(pipe_fd2[0]);
    close(pipe_fd2[1]);
    execvpCommand(commandArray,countWords,0, isAmpersand, isNohup,0);
}
void son2pipe2(char** commandArray,int pipe_fd[],int pipe_fd2[],int countWords,int secondCommand, int isAmpersand[], int isNohup[]){
    int value = dup2(pipe_fd[0],STDIN_FILENO); //duplicate the first pipe to stdin (take input to stdin)
    if (value == -1) {
        fprintf(stderr, "dup2 failed");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        close(pipe_fd2[0]);
        close(pipe_fd2[1]);
        perror("execvp");//if the command didn't work
        freeAllocate(commandArray, countWords);
        exit(1);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    close(pipe_fd2[0]);

    int value1 = dup2(pipe_fd2[1],STDOUT_FILENO); //duplicate the output to the second pipe (send stdout output to the pipe)
    if (value1 == -1) {
        fprintf(stderr, "dup2 failed");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        close(pipe_fd2[0]);
        close(pipe_fd2[1]);
        perror("execvp");//if the command didn't work
        freeAllocate(commandArray, countWords);
        exit(1);
    }
    //close pipes
    close(pipe_fd[1]);
    close(pipe_fd[0]);
    close(pipe_fd2[0]);
    close(pipe_fd2[1]);
    execvpCommand(commandArray,countWords,secondCommand, isAmpersand, isNohup,1);

}
void son3pipe2(char** commandArray,int pipe_fd[],int pipe_fd2[],int countWords,int secondCommand,int thirdCommand, int isAmpersand[], int isNohup[]){
    int value = dup2(pipe_fd2[0],STDIN_FILENO); //duplicate the second pipe to stdin (take input to stdin)
    if (value == -1) {
        fprintf(stderr, "dup2 failed");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        close(pipe_fd2[0]);
        close(pipe_fd2[1]);
        perror("execvp");//if the command didn't work
        freeAllocate(commandArray, countWords);
        exit(1);

    }
    //close pipes
    close(pipe_fd2[1]);
    close(pipe_fd2[0]);
    close(pipe_fd[1]);
    close(pipe_fd[0]);

    execvpCommand(commandArray,countWords,thirdCommand, isAmpersand, isNohup,2);
}

/**
 *method that frees all the allotted space.
 *  @param commandArray-command array
    @param countWords-count how many words there is
**/
void freeAllocate(char** commandArray, const int countWords) {
    if(commandArray==NULL)
        return;
    for (int i = 0; i < (countWords); i++)
        if (commandArray[i] != NULL) {//free the allocated words
            free(commandArray[i]);
            commandArray[i]=NULL;
        }
    if (commandArray != NULL){//free the array
        free(commandArray);
    }
    commandArray=NULL;
}
/**
 *method to the signal that work when the status of one of the sons change and wait
 *  @param a-because the handler method must get an integer

**/
void handler(int a){
    //block all the signals
    sigset_t  mask;
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK,&mask,NULL);

    //wait to the process if exist
    waitpid(-1,NULL,WNOHANG);

    //unblock all the signals
    sigprocmask(SIG_UNBLOCK,&mask,NULL);
}
/**
 *-method that work when the command is nohup command
 * and will allow the program to run even after the SHELL is closed,
 * output will be appended to nohup.txt file and input channel will be closed
 *  @param commandArray-command array
    @param countWords-count how many words there is

**/
void nohupMethod(char** commandArray, const int countWords){
    int fd = open(NOHUP_PATH,
                  O_WRONLY | O_CREAT | O_APPEND ,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    int value = dup2(fd,STDOUT_FILENO);
    if(value == -1){
        fprintf(stderr , "dup2 failed\n");
        freeAllocate(commandArray, countWords);
        exit(1);
    }
    signal(SIGHUP,SIG_IGN);//ignore the hup signal.
    close(STDIN_FILENO);
    close(fd);
}
