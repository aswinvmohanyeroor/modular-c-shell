/**
 * @file command.h
 * @brief Header file defining structures and functions for handling commands and command chains.
 * @version 0.1
 */

#ifndef COMMAND_H
#define COMMAND_H

// Includes
#include "utils.h"
#include <stdbool.h>
#include <unistd.h>

/**
 * @brief Represents a simple command, which includes the command name, arguments, file descriptors, and a function pointer to execute the command.
 * 
 * A simple command consists of a command name and its associated arguments. It may also include input, output, and error file descriptors. Commands are executed in a sequence or pipeline, and I/O redirection is handled by the shell, not by the command itself.
 */
typedef struct SimpleCommand {
    char* commandName; //< Command name, e.g., "ls"

    char** args;       //< Array of arguments for the command, including the command name
    int argc;          //< Number of arguments, including the command name

    int inputFD;       //< Input file descriptor (default is 0 for stdin)
    int outputFD;      //< Output file descriptor (default is 1 for stdout)
    int stderrFD;      //< Error file descriptor (default is 2 for stderr)
    int pid;           //< Process ID of the child process, default is -1
    char* inputFile;   //< Optional input file for redirection
    char* outputFile;  //< Optional output file for redirection
    int noWait;        //< Flag indicating whether to wait for the command to finish (0: wait, 1: no wait)

    int (*execute)(struct SimpleCommand*); //< Function pointer for executing the command
} SimpleCommand;

/**
 * @brief Represents a command, which can be a pipeline of multiple simple commands.
 * 
 * A command is essentially a sequence of simple commands connected by pipes. It also includes information about background execution and the operator used to chain the command with the next one.
 */
typedef struct Command {
    struct SimpleCommand** simpleCommands;  //< Array of pointers to simple commands in this command
    int nSimpleCommands;                    //< Number of simple commands in the array

    bool background;                        //< Flag indicating background execution (true if in background)

    char* chainingOperator;                 //< Operator used to chain this command with the next one (e.g., ';', '&')
    struct Command* next;                   //< Pointer to the next command in the chain
} Command;

/**
 * @brief Represents a chain of commands connected by chaining operators.
 * 
 * A command chain is a sequence of commands linked together using operators such as ';' or '&'. It forms a linked list of commands.
 */
typedef struct CommandChain {
    struct Command* head;   //< Pointer to the first command in the chain
    struct Command* tail;   //< Pointer to the last command in the chain
} CommandChain;

// Function declarations

// ------------------------- Initializers --------------------------------

/**
 * @brief Creates an empty SimpleCommand structure.
 * 
 * Allocates memory for a SimpleCommand and initializes its members to default values. The caller is responsible for freeing the allocated memory.
 * 
 * @return SimpleCommand* Pointer to the newly created SimpleCommand structure, or NULL on failure
 */
SimpleCommand* initSimpleCommand();

/**
 * @brief Creates an empty Command structure.
 * 
 * Allocates memory for a Command and initializes its members to default values. The caller is responsible for freeing the allocated memory.
 * 
 * @return Command* Pointer to the newly created Command structure, or NULL on failure
 */
Command* initCommand();

/**
 * @brief Creates an empty CommandChain structure.
 * 
 * Allocates memory for a CommandChain and initializes its members to default values. The caller is responsible for freeing the allocated memory.
 * 
 * @return CommandChain* Pointer to the newly created CommandChain structure, or NULL on failure
 */
CommandChain* initCommandChain();

// ------------------------- Pushers --------------------------------

/**
 * @brief Adds an argument to the arguments array of a SimpleCommand.
 * 
 * If the command name is not set, it will be set to the provided argument. The function increases the size of the arguments array and updates the argument count.
 * 
 * @param arg Argument to be added
 * @param simpleCommand Pointer to the SimpleCommand structure to which the argument will be added
 * @return int Status code (0 for success, -1 for failure)
 */
int pushArgs(char* arg, SimpleCommand* simpleCommand);

/**
 * @brief Adds a Command to a CommandChain.
 * 
 * Appends the provided command to the end of the chain and updates the tail pointer.
 * 
 * @param chain Pointer to the CommandChain to which the command will be added
 * @param command Pointer to the Command to be added
 * @return int Status code (0 for success, -1 for failure)
 */
int addCommandToChain(CommandChain* chain, Command* command);

/**
 * @brief Adds a SimpleCommand to a Command.
 * 
 * Increases the size of the array holding simple commands in the command and appends the provided SimpleCommand. Uses realloc to adjust the array size.
 * 
 * @param command Pointer to the Command to which the SimpleCommand will be added
 * @param simpleCommand Pointer to the SimpleCommand to be added
 * @return int Status code (0 for success, -1 for failure)
 */
int addSimpleCommand(Command* command, SimpleCommand* simpleCommand);

// ------------------------- Cleaners --------------------------------

/**
 * @brief Frees the memory allocated for a SimpleCommand.
 * 
 * Deallocates memory for the command name, arguments array, and any associated files. Sets the pointer to NULL.
 * 
 * @param simpleCommand Pointer to the SimpleCommand structure to be cleaned up
 */
void cleanUpSimpleCommand(SimpleCommand* simpleCommand);

/**
 * @brief Frees the memory allocated for a Command.
 * 
 * Deallocates memory for the array of SimpleCommand pointers and the chaining operator string. Sets the pointer to NULL.
 * 
 * @param command Pointer to the Command structure to be cleaned up
 */
void cleanUpCommand(Command* command);

/**
 * @brief Frees the memory allocated for a CommandChain.
 * 
 * Deallocates memory for all commands in the chain and updates the head and tail pointers to NULL.
 * 
 * @param chain Pointer to the CommandChain structure to be cleaned up
 */
void cleanUpCommandChain(CommandChain* chain);

// ------------------------- Execute --------------------------------

/**
 * @brief Executes a chain of commands.
 * 
 * Traverses the command chain and executes each command in sequence. Handles chaining operators to determine execution order. Returns the exit status of the last command.
 * 
 * @param chain Pointer to the CommandChain to be executed
 * @return int Status code (exit status of the last command)
 */
int executeCommandChain(CommandChain* chain);

/**
 * @brief Executes a Command.
 * 
 * Traverses and executes each SimpleCommand in the Command. Manages the execution sequence and returns the exit status of the last executed command.
 * 
 * @param command Pointer to the Command to be executed
 * @return int Status code (exit status of the last command)
 */
int executeCommand(Command* command);

// ------------------------- Debug --------------------------------

/**
 * @brief Prints the details of a CommandChain in a human-readable format.
 * 
 * Useful for debugging to visualize the structure of the command chain.
 * 
 * @param chain Pointer to the CommandChain to be printed
 */
void printCommandChain(CommandChain* chain);

/**
 * @brief Prints the details of a SimpleCommand in a human-readable format.
 * 
 * Useful for debugging to visualize the details of the simple command.
 * 
 * @param simpleCommand Pointer to the SimpleCommand to be printed
 */
void printSimpleCommand(SimpleCommand* simpleCommand);

#endif // COMMAND_H
