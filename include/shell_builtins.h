/**
 * @file builtins.h
 * @brief Contains structures and utilities for managing built-in commands and the internal state of the shell.
 * @version 0.1
 * 
 * This file includes definitions for shell history management, shell state representation, and built-in command functions.
 * 
 */

#ifndef BUILTINS_H
#define BUILTINS_H

#include "command.h"

// Maximum path length for file operations
#define MAX_PATH_LENGTH 1024

// Environment variable for home directory
#define HOME_DIR getenv("HOME")

// Structure to represent a single command in the shell history
typedef struct HistoryNode {
    char* command;                /**< The command string */
    struct HistoryNode* next;     /**< Pointer to the next node in the history list */
} HistoryNode;

// Structure to represent a list of commands in shell history
typedef struct HistoryList {
    HistoryNode* head;            /**< Pointer to the first node in the history list */
    HistoryNode* tail;            /**< Pointer to the last node in the history list for quick insertions */
    size_t size;                  /**< The number of commands in the history list */
} HistoryList;

/**
 * @brief Adds a command to the history list.
 * 
 * This function creates a new HistoryNode and appends it to the history list. The command string is copied to the new node.
 * 
 * @param list The history list to which the command will be added.
 * @param command The command string to add.
 * @return int Returns 0 on success, -1 on failure.
 */
int add_to_history(HistoryList* list, char* command);

/**
 * @brief Retrieves a command at a particular index from the history list.
 * 
 * This function returns the command string at the specified index in the history list.
 * 
 * @param list The history list from which to retrieve the command.
 * @param index The index of the command to retrieve.
 * @return char* The command string at the specified index, or NULL if the index is out of range.
 */
char* get_command(HistoryList* list, unsigned int index);

/**
 * @brief Cleans up and frees memory used by the history list.
 * 
 * This function frees all nodes in the history list and the memory associated with them.
 * 
 * @param list The history list to be cleaned up.
 */
void clean_history(HistoryList* list);

/**
 * @brief Finds the last command in the history that starts with the given prefix.
 * 
 * This function searches through the history list and returns the most recent command that starts with the specified prefix.
 * 
 * @param list The history list to search.
 * @param prefix The prefix to search for.
 * @return char* The last command that starts with the prefix, or NULL if no match is found.
 */
char* find_last_command_with_prefix(HistoryList* list, const char* prefix);

// Structure to represent the state of the shell
typedef struct ShellState {

    // File descriptors for the original standard input, output, and error
    int originalStdoutFD;  /**< File descriptor for original stdout */
    int originalStdinFD;   /**< File descriptor for original stdin */
    int originalStderrFD;  /**< File descriptor for original stderr */

    // Buffer to hold the current prompt string
    char prompt_buffer[MAX_STRING_LENGTH]; /**< Current shell prompt string */

    // History list to store commands entered by the user
    HistoryList history;  /**< The shell command history list */
} ShellState;

/**
 * @brief Initializes the shell state.
 * 
 * This function allocates and initializes a ShellState structure, setting up the original file descriptors and prompt buffer.
 * 
 * @return ShellState* Pointer to the initialized ShellState structure, or NULL on failure.
 */
ShellState* init_shell_state();

/**
 * @brief Cleans up and frees memory used by the shell state.
 * 
 * This function frees all resources associated with the ShellState structure, including the history list.
 * 
 * @param stateObj The ShellState structure to be cleaned up.
 * @return int Returns 0 on success, -1 on failure.
 */
int clear_shell_state(ShellState* stateObj);

/**
 * @brief Type definition for a function that executes a simple command.
 * 
 * This type is used for function pointers that refer to built-in command execution functions.
 * 
 * @param command The SimpleCommand structure representing the command to execute.
 * @return int Returns 0 on success, or a non-zero status on failure.
 */
typedef int (*ExecutionFunction)(SimpleCommand*);

/**
 * @brief Returns the execution function for a given built-in command.
 * 
 * This function returns a pointer to the function that handles the execution of the specified command.
 * 
 * @param commandName The name of the command.
 * @return ExecutionFunction The function pointer for executing the command.
 */
ExecutionFunction getExecutionFunction(char* commandName);

/**
 * @brief Built-in function to change the current directory.
 * 
 * This function changes the current working directory to the path specified in the command.
 * 
 * @param command The command to be executed, which should include the target directory.
 * @return int Returns 0 on success, -1 on failure.
 */
int cd(SimpleCommand* command);

/**
 * @brief Built-in function to exit the shell.
 * 
 * This function terminates the shell process.
 * 
 * @param command The command to be executed, which should be the exit command.
 * @return int Returns 0 on success, -1 on failure.
 */
int exitShell(SimpleCommand* command);

/**
 * @brief Built-in function to print the current working directory.
 * 
 * This function outputs the current working directory to the standard output.
 * 
 * @param command The command to be executed, which should be the pwd command.
 * @return int Returns 0 on success, -1 on failure.
 */
int pwd(SimpleCommand* command);

/**
 * @brief Built-in function to print the command history.
 * 
 * This function outputs the command history stored in the shell's history list.
 * 
 * @param command The command to be executed, which should be the history command.
 * @return int Returns 0 on success, -1 on failure.
 */
int history(SimpleCommand* command);

/**
 * @brief Executes a process by forking and executing the command.
 * 
 * This function creates a child process, executes the command in the child process, and returns the execution status.
 * 
 * @param command The command to be executed.
 * @return int Returns 0 on success, non-zero on failure.
 */
int executeProcess(SimpleCommand* command);

#endif // BUILTINS_H
