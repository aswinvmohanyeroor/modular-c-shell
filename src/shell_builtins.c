/**
 * @file builtins.c
 * @brief Contains the function definitions for the builtin shell functions.
 * @version 0.1
 * 
 */

#include "shell_builtins.h"
#include "parser.h"
#include "command.h"

#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

// Global variable to store the shell's state
extern ShellState* globalShellState;

/*-------------------------------History Management----------------------------------*/

/**
 * @brief Adds a command to the history list.
 * 
 * Allocates memory for a new HistoryNode, sets its command to the provided command string, and updates the history list's head and tail pointers.
 * 
 * @param list The history list to which the command should be added.
 * @param command The command string to be added to the history list.
 * @return int Status code (0 on success, -1 on failure).
 */
int add_to_history(HistoryList* list, char* command)
{
    if (!command)
        return -1;

    HistoryNode* node = malloc(sizeof(HistoryNode));
    node->command = COPY(command);
    node->next = NULL;

    if (!list->head)
    {
        list->head = node;
    }
    else if (!list->head->next)
    {
        list->tail = node;
        list->head->next = list->tail;
    }
    else
    {
        list->tail->next = node;
        list->tail = node;
    }

    list->size++;
    return 0;
}

/**
 * @brief Retrieves a command at a particular index from the history list.
 * 
 * Traverses the history list to find and return the command at the specified index.
 * 
 * @param list The history list.
 * @param index The index of the command to retrieve.
 * @return char* The command at the specified index or NULL if not found.
 */
char* get_command(HistoryList* list, unsigned int index)
{
    if (index > list->size || !list->head)
        return NULL;

    HistoryNode* curr = list->head;
    unsigned int ctr = 1;

    for (; curr && ctr != index; curr = curr->next, ctr++);

    return curr->command;    
}

/**
 * @brief Cleans up the history list by freeing all allocated memory.
 * 
 * Iterates through the history list, frees each node and its command, and resets the list.
 * 
 * @param list The history list to clean up.
 */
void clean_history(HistoryList* list) {
    HistoryNode* current = list->head;
    HistoryNode* next;

    while (current != NULL) {
        next = current->next;
        free(current->command);
        free(current);
        current = next;
    }

    // After cleaning up all nodes, reset the list
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/**
 * @brief Finds the last command in the history that starts with the specified prefix.
 * 
 * Searches the history list for the most recent command that starts with the given prefix.
 * 
 * @param list The history list.
 * @param prefix The prefix to match commands against.
 * @return char* The most recent command matching the prefix or NULL if no match is found.
 */
char* find_last_command_with_prefix(HistoryList* list, const char* prefix)
{
    if (list == NULL || list->head == NULL || prefix == NULL) {
        return NULL; // Invalid input
    }

    HistoryNode* current = list->head;
    char* lastCommand = NULL;

    while (current != NULL) 
    {
        // Check if the current command starts with the given prefix
        if (strncmp(current->command, prefix, strlen(prefix)) == 0) {
            // Update the lastCommand whenever a match is found
            lastCommand = current->command; // Duplicate the string
        }

        current = current->next;
    }

    return lastCommand;
}

/*-------------------------------Shell State Management----------------------------------*/

/**
 * @brief Initializes the shell state with default values.
 * 
 * Allocates memory for a ShellState object, sets default file descriptors, and initializes the history list.
 * 
 * @return ShellState* Pointer to the initialized ShellState object.
 */
ShellState* init_shell_state()
{
    ShellState* stateObj = malloc(sizeof(ShellState));
    if (!stateObj)
    {
        LOG_ERROR("malloc failure. Exiting.\n");
        exit(-1);
    }

    stateObj->originalStdinFD = STDIN_FD;
    stateObj->originalStdoutFD = STDOUT_FD;
    stateObj->originalStderrFD = STDERR_FD;

    // default prompt
    strncpy(stateObj->prompt_buffer, "\%", MAX_STRING_LENGTH);

    stateObj->history.head = NULL;
    stateObj->history.tail = NULL;
    stateObj->history.size = 0;

    return stateObj;
}

/**
 * @brief Frees memory allocated for the shell state.
 * 
 * Frees the ShellState object and resets its pointer.
 * 
 * @param stateObj The ShellState object to be cleared.
 * @return int Status code (0 on success, -1 on failure).
 */
int clear_shell_state(ShellState* stateObj)
{
    if (!stateObj)
    {
        LOG_DEBUG("Can't clear NULL shell state object.\n");
        return -1;
    }

    free(stateObj);
    return 0;
}

/*-------------------------------File Descriptor Management----------------------------------*/

/**
 * @brief Sets up file descriptors for input, output, and stderr.
 * 
 * Uses the `dup2` system call to duplicate file descriptors for input, output, and stderr if they differ from the default values.
 * 
 * @param inputFD The input file descriptor.
 * @param outputFD The output file descriptor.
 * @param stderrFD The stderr file descriptor.
 * @return int Status code (0 on success, -1 on failure).
 */
static int setUpFD(int inputFD, int outputFD, int stderrFD)
{
    if (inputFD != STDIN_FD)
    {
        globalShellState->originalStdinFD = dup(STDIN_FD);

        if (dup2(inputFD, STDIN_FD) == -1)
        {
            LOG_DEBUG("dup2: %s\n", strerror(errno));
            return -1;
        }

        close(inputFD);
    }

    if (outputFD != STDOUT_FD)
    {
        globalShellState->originalStdoutFD = dup(STDOUT_FD);
        
        if (dup2(outputFD, STDOUT_FD) == -1)
        {
            LOG_DEBUG("dup2: %s\n", strerror(errno));
            return -1;
        }

        close(outputFD);
    }

    if (stderrFD != STDERR_FD)
    {
        globalShellState->originalStderrFD = dup(STDERR_FD);
        
        if (dup2(stderrFD, STDERR_FD) == -1)
        {
            LOG_DEBUG("dup2: %s\n", strerror(errno));
            return -1;
        }

        close(stderrFD);
    }

    return 0;
}

/**
 * @brief Resets file descriptors to their original values.
 * 
 * Restores the original file descriptors for input, output, and stderr.
 */
static void resetFD()
{
    if (globalShellState->originalStdinFD != STDIN_FD)
    {
        if (dup2(globalShellState->originalStdinFD, STDIN_FD) == -1)
        {
            LOG_ERROR("dup2: %s\n", strerror(errno));
            exit(1);
        }
    }

    if (globalShellState->originalStdoutFD != STDOUT_FD)
    {
        if (dup2(globalShellState->originalStdoutFD, STDOUT_FD) == -1)
        {
            LOG_ERROR("dup2: %s\n", strerror(errno));
            exit(1);
        }
    }

    if (globalShellState->originalStderrFD != STDERR_FD)
    {
        if (dup2(globalShellState->originalStderrFD, STDERR_FD) == -1)
        {
            LOG_ERROR("dup2: %s\n", strerror(errno));
            exit(1);
        }
    }
}

/*-------------------------------Builtin Commands----------------------------------*/

/**
 * @brief Changes the current directory.
 * 
 * Changes the working directory to the specified path or to the home directory if no path is provided.
 * 
 * @param simpleCommand The command to execute, containing the arguments for `cd`.
 * @return int Status code (0 on success, -1 on failure).
 */
int cd(SimpleCommand* simpleCommand)
{
    if (simpleCommand->argc > 2)
    {
        LOG_ERROR("cd: Too many arguments\n");
        return -1;
    }

    // Don't think cd ever needs any input from stdin, neither it puts anything to stdout, so dont need to modify file descriptors
    const char* path = NULL;
    if (simpleCommand->argc == 1)
    {
        // No path specified, go to home directory
        path = HOME_DIR;
    }
    else
    {
        path = simpleCommand->args[1];
    }

    if (chdir(path) == -1)
    {
        LOG_ERROR("cd: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * @brief Prints the current working directory.
 * 
 * Retrieves and prints the current working directory to stdout.
 * 
 * @param simpleCommand The command to execute, expected to have no arguments.
 * @return int Status code (0 on success, -1 on failure).
 */
int pwd(SimpleCommand* simpleCommand)
{
    if (simpleCommand->argc > 1)
    {
        LOG_ERROR("pwd: Too many arguments\n");
        return -1;
    }

    char cwd[MAX_PATH_LENGTH];

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        LOG_ERROR("pwd: %s\n", strerror(errno));
        return -1;
    }

    if (setUpFD(simpleCommand->inputFD, simpleCommand->outputFD, simpleCommand->stderrFD))
    {
        return -1;
    }

    LOG_PRINT("%s\n", cwd);

    resetFD();
    return 0;
}

/**
 * @brief Exits the shell with an optional status code.
 * 
 * Terminates the shell process with the provided exit status or with status 0 if no status is provided.
 * 
 * @param simpleCommand The command to execute, which may include an optional exit status.
 * @return int Status code (0 on success, -1 on failure).
 */
int exitShell(SimpleCommand* simpleCommand)
{
    if (simpleCommand->argc > 2)
    {
        printf("exit: Too many arguments\n");
        return -1;
    }
    LOG_OUT("exit\n");

    if (simpleCommand->argc == 1)
        exit(0);
    
    // check if each char in the second arg is a number or not. that was the only standard compliant way I could think of to figure whether the argument is a numebr or not
    if(strspn(simpleCommand->args[1], "0123456789") != strlen(simpleCommand->args[1]))
    {
        LOG_ERROR("exit: Expects a numerical argument\n");
        return -1;
    }

    int exit_status = atoi(simpleCommand->args[1]);
    exit(exit_status);
}

/**
 * @brief Displays the command history or executes a command from history.
 * 
 * If no arguments are provided, prints the entire history list. If an index or prefix is provided, executes the corresponding command from the history.
 * 
 * @param simpleCommand The command to execute, which may include arguments for history retrieval or command execution.
 * @return int Status code (0 on success, -1 on failure).
 */
int history(SimpleCommand* simpleCommand)
{
    if (simpleCommand->argc > 2)
    {
        LOG_ERROR("history: Too many arguments\n");
        return -1;
    }

    if (setUpFD(simpleCommand->inputFD, simpleCommand->outputFD, simpleCommand->stderrFD))
    {
        return -1;
    }

    if (simpleCommand->argc == 1)
    {
        HistoryNode* curr = globalShellState->history.head;
        int i = 1;
        while (curr)
        {
            LOG_PRINT("%d %s\n", i, curr->command);
            curr = curr->next;
            i++;
        }

        resetFD();
    }
    else
    {
        char* input = NULL;
        if(strspn(simpleCommand->args[1], "0123456789") == strlen(simpleCommand->args[1]))
        {
            // execute the command at that index
            unsigned int idx = (unsigned int)atoi(simpleCommand->args[1]);
            input = COPY(get_command(&globalShellState->history, idx));

            if (!input)
            {
                LOG_ERROR("history: invalid index\n");
                return -1;
            }
        }
        else
        {
            char* last = find_last_command_with_prefix(&globalShellState->history, simpleCommand->args[1]);
            input = COPY(last);

            if (!input)
            {
                LOG_ERROR("history: no matching command found\n");
                return -1;
            }
        }

        // simple whitespace tokenizer
        char** tokens = tokenizeString(input, ' ');

        // generate the command from tokens
        CommandChain* commandChain = parseTokens(tokens);
        
        // execute the command
        int status = executeCommandChain(commandChain);
        (void)status;
        // Free tokens
        freeTokens(tokens);

        // free the command chain
        cleanUpCommandChain(commandChain);

        // Free buffer that was allocated for input
        free(input);
    }

    return 0;
}

/**
 * @brief Executes a simple command in a child process.
 * 
 * Forks a new process, sets up file descriptors, executes the command, and optionally waits for the child process to finish.
 * 
 * @param simpleCommand The command to execute, including its arguments and file descriptors.
 * @return int Status code (0 on success, -1 on failure).
 */
int executeProcess(SimpleCommand* simpleCommand)
{
    int pid = fork();

    if (pid == -1)
    {
        LOG_DEBUG("fork: %s\n", strerror(errno));
        return -1;
    }
    else if (pid == 0)
    {
        // Child process
        setUpFD(simpleCommand->inputFD, simpleCommand->outputFD, simpleCommand->stderrFD);

        // Execute the command
        if (execvp(simpleCommand->commandName, simpleCommand->args) == -1)
        {
            LOG_ERROR("%s: %s\n", simpleCommand->commandName, strerror(errno));
            exit(1);
        }

        // This should never be reached
        LOG_ERROR("This should never be reached\n");
        exit(0);
    }
    else
    {
        // Parent process
        simpleCommand->pid = pid;

        if (!simpleCommand->noWait) {
            // Wait for the child process to finish
            int status;
            LOG_DEBUG("Waiting for child process, with command name %s\n", simpleCommand->commandName);
            if (waitpid(pid, &status, 0) == -1)
            {
                LOG_ERROR("waitpid: %s\n", strerror(errno));
                return -1;
            }

            // Print the exit status of the child process
            if (WEXITSTATUS(status) != 0)
            {
                LOG_DEBUG("Non zero exit status : %d\n", WEXITSTATUS(status));
                return WEXITSTATUS(status);
            }
        }
    }

    LOG_DEBUG("Finished executing command %s\n", simpleCommand->commandName);
    return 0;
}

/**
 * @brief Changes the current prompt of the shell.
 * 
 * Sets the shell prompt to the specified string.
 * 
 * @param simpleCommand The command to execute, including the new prompt string.
 * @return int Status code (0 on success, -1 on failure).
 */
int prompt(SimpleCommand* simpleCommand) 
{
    if (simpleCommand->argc == 1)
    {
        LOG_ERROR("prompt: Too few arguments\n");
        return -1;
    }

    if (simpleCommand->argc > 2)
    {
        LOG_ERROR("prompt: Too many arguments\n");
        return -1;
    }

    strcpy(globalShellState->prompt_buffer, simpleCommand->args[1]);

    return 0;
}

/*-------------------------------Command Registry----------------------------------*/

/**
 * @brief Represents a builtin command and its corresponding execution function.
 * 
 * Holds the name of the command and a pointer to its execution function.
 */
typedef struct commandRegistry
{
    char* commandName;
    ExecutionFunction executionFunction;
} CommandRegistry;

/**
 * @brief Registry of all the builtin commands and their execution functions.
 * 
 * Maps command names to their corresponding execution functions. If a command is not found in the registry, the default function `executeProcess` is used.
 * 
 * @return ExecutionFunction Pointer to the function to execute for the given command.
 */
static const CommandRegistry commandRegistry[] = {
    {"cd", cd},
    {"pwd", pwd},
    {"exit", exitShell},
    {"history", history},
    {"prompt", prompt},
    {NULL, NULL}
};

/**
 * @brief Retrieves the execution function for a given command name.
 * 
 * Searches the command registry for the specified command and returns its associated execution function. If the command is not found, returns the default process execution function.
 * 
 * @param commandName The name of the command to look up.
 * @return ExecutionFunction Pointer to the execution function for the command.
 */
ExecutionFunction getExecutionFunction(char* commandName)
{
    for (int i = 0; commandRegistry[i].commandName != NULL; i++)
    {
        if (strcmp(commandRegistry[i].commandName, commandName) == 0)
        {
            return commandRegistry[i].executionFunction;
        }
    }

    return executeProcess;
}
