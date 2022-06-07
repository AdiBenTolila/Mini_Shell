# Mini_Shell
command prompt, receives commands and executes them
Mini shell
Authored by Adi Bentolila

# Description
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
 * invalid input-if in the start or in the end of the string there is a space or the string is just enter
 * The maximum string length is 512 characters.
 * There is no limit to the number of words (as long as the string is in the limit length).
 * There is no limit to the number of characters of a word (as long as the string is in the limit length).
 * string cant start or end with pipe
 * There can be several spaces between one word and another, but it can be assumed that there is no other character between them (like '\ r', '\ t').
 * the string can't be with more than 2 pipes
 * if we have invalid command but in the last command there is an valid command the command will be executed and her output will be printed in addition to error messege
  as in the original shell
 * the program count pipe even if its more than 2 pipes but dont count if its in quotationMark

## functions:
1. makePath-A method that gets the name of the current folder
2. numOfWord-method that count how many words there is to allocate and add to the total.
3. doCommandHistory-A method that put the new command from the history in the inputCommand
4. checkFromHistory-A method that check if there is exclamation mark and put the new command from the history in the right place
5. stringAnalysis-method that reserved a command and put the command in the array command
6. insert-method that insert word from the method stringAnalysis and creat the array of all the words in the command.
7. writToFile-method that write to the file the sentence that reserved
8. readFromFile-method that open the file and print all the data inside - when the user insert the word "history".
9. executeOneCommand-method that creates the new process.
10. execute-method that creates all the new process with the pipe.
11. execvpCommand-method that do the execvp and check before if the command is history or have nohup
12. freeAllocate-method that frees all the allotted space.
13. handler-method to the signal that work when the status of one of the sons change and wait
14. nohupMethod-method that work when the command is nohup command and will allow the program to run even after the SHELL is closed, output will be appended to nohup.txt file and input channel will be closed

the following function  indicates to the shell to route the STDOUT of the command before it to the STDIN of the command following it:
son1pipe1
son2pipe1
son1pipe2
son2pipe2
son3pipe2


## Program Files
file.txt- the file contain all the history of the strings that the user enters
nohup.txt - the file contain all nohup commands output
ex3.c- contain the program (Receives an commands and executes it accordingly and use the commands history by:!).

## How to compile?
compile: 
```[bash]
gcc shell.c -o shell
```
run:
```[bash]
./shell
```

## Input:
the user insert command

## Output:
- command output or error
- number of the commnd and also number of the pipe(when the user enter:"done")
- file.txt will be created
- nohup.txt will be created with nohup commands output
- print all the data in the file(when the user enter:"history")
