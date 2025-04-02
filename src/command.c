/**
 * @file command.c
 * @brief Contains the function implementations for handling commands in the shell.
 * @version 0.1
 * 
 * This file defines functions for initializing, managing, executing, and cleaning up commands and command chains.
 * 
 */

#include "command.h"

// Macro to check if the previous command is chained with a specific operator
#define CHAINED_WITH(opr) (prevCommand ? (prevCommand->chainingOperator ? (strcmp(prevCommand->chainingOperator, opr) == 0) : 0) : 0)

/*----------------------------------------------------------------------------------------*/

/*-------------------------------Initializers--------------------------------------------*/

// Initializes a SimpleCommand structure with default values
SimpleCommand* initSimpleCommand()
{
    SimpleCommand* simpleCommand = (SimpleCommand*) malloc(sizeof(SimpleCommand));

    if (!simpleCommand)
        return NULL;  // Return NULL if memory allocation fails
    
    // Set default values for the SimpleCommand fields
    simpleCommand->commandName = NULL;
    simpleCommand->args        = NULL;
    simpleCommand->argc        = 0;
    simpleCommand->inputFD     = STDIN_FD;
    simpleCommand->outputFD    = STDOUT_FD;
    simpleCommand->stderrFD    = STDERR_FD;
    simpleCommand->noWait      = 0;
    simpleCommand->execute     = NULL;
    simpleCommand->pid         = -1;

    return simpleCommand;
}

// Initializes a Command structure with default values
Command* initCommand()
{
    Command* command = (Command*)malloc(sizeof(Command));

    if (!command)
        return NULL;  // Return NULL if memory allocation fails

    // Set default values for the Command fields
    command->simpleCommands   = NULL;
    command->nSimpleCommands  = 0;
    command->background       = false;
    command->chainingOperator = NULL;
    command->next             = NULL;

    return command;
}

// Initializes a CommandChain structure with default values
CommandChain* initCommandChain()
{
    CommandChain* chain = (CommandChain*)malloc(sizeof(CommandChain));

    if (!chain)
        return NULL;  // Return NULL if memory allocation fails

    // Set default values for the CommandChain fields
    chain->head = NULL;
    chain->tail = NULL;

    return chain;
}

/*-------------------------------Setters (push functions)--------------------------------*/

// Adds a Command to the end of the CommandChain
int addCommandToChain(CommandChain* chain, Command* command)
{
    if (!chain)
    {
        LOG_DEBUG("Invalid command chain passed\n");
        return -1;  // Return error code if chain is NULL
    }

    // If the chain is empty, set both head and tail to the new command
    if (!chain->head)
    {
        chain->head = command;
        chain->tail = command;
    }
    // Otherwise, append the command to the end of the chain
    else
    {
        chain->tail->next = command;
        chain->tail = command;
    }

    return 0;  // Return success code
}

// Adds a SimpleCommand to the current Command's array of SimpleCommands
int addSimpleCommand(Command* command, SimpleCommand* simpleCommand)
{
    if (!command)
    {
        LOG_DEBUG("Invalid command passed. It's NULL\n");
        return -1;  // Return error code if command is NULL
    }

    if (!simpleCommand)
    {
        LOG_DEBUG("Invalid simpleCommand passed. It's NULL\n");
        return -1;  // Return error code if simpleCommand is NULL
    }

    // Reallocate memory to accommodate the new SimpleCommand
    SimpleCommand** temp = (SimpleCommand**)realloc(command->simpleCommands, (command->nSimpleCommands + 1) * sizeof(SimpleCommand*));

    if (!temp)
    {
        LOG_DEBUG("Realloc error. Failed to reallocate memory for the array.\n");
        return -1;  // Return error code if reallocation fails
    }

    command->simpleCommands = temp;
    temp = NULL;

    // Add the new SimpleCommand to the end of the array
    command->simpleCommands[command->nSimpleCommands] = simpleCommand;
    command->nSimpleCommands++;  // Increment the count of SimpleCommands

    return 0;  // Return success code
}

// Adds an argument to the SimpleCommand's argument array, ensuring it's NULL-terminated
int pushArgs(char* arg, SimpleCommand* simpleCommand)
{
    if (!simpleCommand)
    {
        LOG_DEBUG("Invalid simpleCommand passed. It's NULL\n");
        return -1;  // Return error code if simpleCommand is NULL
    }

    // Reallocate memory to accommodate the new argument
    char** temp = (char**)realloc(simpleCommand->args, (simpleCommand->argc + 2) * sizeof(char*));

    if (!temp)
    {
        LOG_DEBUG("Realloc error. Failed to reallocate memory for the array.\n");
        return -1;  // Return error code if reallocation fails
    }

    simpleCommand->args = temp;
    temp = NULL;

    // Add the new argument to the end of the array and ensure the array is NULL-terminated
    simpleCommand->args[simpleCommand->argc] = COPY(arg);
    simpleCommand->args[simpleCommand->argc + 1] = NULL;
    simpleCommand->argc++;

    // Set the command name if this is the first argument
    if (simpleCommand->argc == 1)
    {
        simpleCommand->commandName = COPY(arg);
    }

    return 0;  // Return success code
}

/*-------------------------------Command Execution functions------------------------------*/

// Executes a CommandChain, processing each Command in sequence
int executeCommandChain(CommandChain* chain)
{
    if (!chain)
    {
        LOG_DEBUG("Invalid command chain passed\n");
        return -1;  // Return error code if chain is NULL
    }

    Command* command = chain->head;

    // Variable to hold the exit status of the last command
    int lastStatus = 0;

    // Process each Command in the chain
    while (command)
    {   
        lastStatus = executeCommand(command);  // Execute the current command

        command = command->next;  // Move to the next command in the chain
    }

    return lastStatus;  // Return the exit status of the last command
}

// Executes a single Command, including I/O redirections
int executeCommand(Command* command)
{
    if (!command)
    {
        LOG_DEBUG("Invalid command passed\n");
        return -1;  // Return error code if command is NULL
    }

    // Check if the command is empty and return an error if so
    if (!command->simpleCommands || command->nSimpleCommands == 0)
    {
        LOG_DEBUG("Invalid command. It's empty\n");
        return -1;  // Return error code if command is empty
    }

    for (int i = 0; i < command->nSimpleCommands; i++)
    {
        LOG_DEBUG("Executing command : %s\n", command->simpleCommands[i]->commandName);
        SimpleCommand* simpleCommand = command->simpleCommands[i];

        // If the command is to be run in the background, set noWait flag
        if (command->background)
            simpleCommand->noWait = 1;

        // Check if the command name is empty and return an error if so
        if (!simpleCommand->commandName)
        {
            LOG_DEBUG("Invalid command name. It's empty\n");
            return -1;  // Return error code if command name is empty
        }
        
        // Execute the SimpleCommand and get the status
        int status = simpleCommand->execute(simpleCommand);
        LOG_DEBUG("Command executing with pid: %d\n", simpleCommand->pid);

        // If the command execution failed, return the status
        if (status)
        {
            return status;
        }

        // Close file descriptors if they were redirected
        if (simpleCommand->inputFD != STDIN_FD)
            close(simpleCommand->inputFD);
        
        if (simpleCommand->outputFD != STDOUT_FD)
            close(simpleCommand->outputFD);
    }

    return 0;  // Return success code
}

/*-------------------------------Clean up functions---------------------------------------*/

// Frees memory allocated for a SimpleCommand
void cleanUpSimpleCommand(SimpleCommand* simpleCommand)
{
    if (!simpleCommand)
        return;  // Return if the SimpleCommand is NULL

    LOG_DEBUG("Cleaning up simple command: %s\n", simpleCommand->commandName);

    // Free the commandName if it was allocated
    if (simpleCommand->commandName)
    {
        free(simpleCommand->commandName);
        simpleCommand->commandName = NULL;
    }

    // Free each argument in the args array
    if (simpleCommand->args)
    {
        for (int i = 0; i < simpleCommand->argc; i++)
        {
            if (simpleCommand->args[i])
            {
                free(simpleCommand->args[i]);
                simpleCommand->args[i] = NULL;
            }
        }

        // Free the args array itself
        free(simpleCommand->args);
        simpleCommand->args = NULL;
    }

    // Free the SimpleCommand structure
    free(simpleCommand);
    simpleCommand = NULL;
}

// Frees memory allocated for a Command
void cleanUpCommand(Command* command)
{
    if (!command)
        return;  // Return if the Command is NULL

    // Free each SimpleCommand in the Command
    if (command->simpleCommands)
    {
        for (int i = 0; i < command->nSimpleCommands; i++)
        {
            cleanUpSimpleCommand(command->simpleCommands[i]);
        }
    }

    // Free the array of SimpleCommands
    free(command->simpleCommands);

    // Free the chainingOperator if it was allocated
    if (command->chainingOperator)
    {
        free(command->chainingOperator);
        command->chainingOperator = NULL;
    }

    // No need to free next command, as it will be handled by chain cleanup
}

// Frees memory allocated for the CommandChain and its commands
void cleanUpCommandChain(CommandChain* chain)
{
    if (!chain)
        return;  // Return if the CommandChain is NULL

    // Free each Command in the CommandChain
    if (chain->head)
    {
        Command* command = chain->head;
        while (command)
        {
            Command* nextCommand = command->next;
            cleanUpCommand(command);
            free(command);
            command = nextCommand;
        }
    }

    // Free the CommandChain structure
    free(chain);
    chain = NULL;
}

/*-------------------------------Utility functions----------------------------------------*/

// Prints the details of a CommandChain
void printCommandChain(CommandChain* chain)
{
    LOG_DEBUG("Printing command chain\n");
    if (!chain)
        return;  // Return if the CommandChain is NULL

    Command* command = chain->head;
    int counter = 1;
    while (command)
    {
        LOG_DEBUG("[Link %d]\n", counter);
        for (int i = 0; i < command->nSimpleCommands; i++)
        {
            printSimpleCommand(command->simpleCommands[i]);
        }
        counter++;
        command = command->next;
    }
}

// Prints the details of a SimpleCommand
void printSimpleCommand(SimpleCommand* simpleCommand)
{
    if (!simpleCommand)
        return;  // Return if the SimpleCommand is NULL

    LOG_DEBUG("-- name: %s\n", simpleCommand->commandName);
    LOG_DEBUG("-- args:\n");
    for (int i = 0; i < simpleCommand->argc; i++)
    {
        LOG_DEBUG("-- -- %s \n", simpleCommand->args[i]);
    }

    LOG_DEBUG("-- Input FD: %d\n", simpleCommand->inputFD);
    LOG_DEBUG("-- Output FD: %d\n", simpleCommand->outputFD);
    LOG_DEBUG("--------------------\n");
}

/*----------------------------------------------------------------------------------------*/
