/**
 * @file utils.h
 * @brief Contains useful macros and utility functions for the shell.
 * @version 0.1
 * 
 * This header file includes macros for logging, utility functions for handling strings and file descriptors,
 * and other utility functions useful for shell operations.
 * 
 * 
 */

#ifndef UTILS_H
#define UTILS_H

#include "log.h"

#include <string.h>
#include <stdlib.h>

// Macros for logging with different levels and colors
#define LOG_ERROR(...) LOG(LOG_ERR, "[ERROR]", LOG_COLOR_ERR, __VA_ARGS__)   /**< Macro for logging error messages */
#define LOG_DEBUG(...) LOG(LOG_DBG, "[DEBUG]", LOG_COLOR_DBG, __VA_ARGS__)   /**< Macro for logging debug messages */
#define LOG_PRINT(...) LOG(LOG_PRI, "[PRINT]", LOG_COLOR_PRI, __VA_ARGS__)   /**< Macro for logging general print messages */

// Defines the maximum length for strings used in the program
#define MAX_STRING_LENGTH 1024   /**< Maximum length of strings (including null terminator) */

// Macro for safely copying a string up to MAX_STRING_LENGTH
#define COPY(str) (str ? strndup(str, MAX_STRING_LENGTH) : NULL)  /**< Macro to duplicate a string up to a maximum length, returns NULL if input is NULL */

// File descriptor constants for standard input, output, and error, and for pipe ends
#define STDIN_FD 0                /**< File descriptor for standard input */
#define STDOUT_FD 1               /**< File descriptor for standard output */
#define STDERR_FD 2               /**< File descriptor for standard error */
#define PIPE_READ_END 0          /**< Pipe end for reading data */
#define PIPE_WRITE_END 1         /**< Pipe end for writing data */

/**
 * @brief Tokenizes a string based on a delimiter.
 * 
 * This function splits a string into an array of tokens, using the specified delimiter. It handles quoted strings properly, 
 * ignoring delimiters within quotes. The resulting array is NULL-terminated.
 * 
 * @param str The string to tokenize.
 * @param delimiter The character used to delimit tokens.
 * @return char** An array of tokens (NULL terminated). The caller is responsible for freeing this memory using freeTokens().
 */
char** tokenizeString(const char* str, const char delimiter);

/**
 * @brief Frees the memory allocated for tokens.
 * 
 * This function frees the memory used by the array of tokens returned by tokenizeString().
 * 
 * @param tokens The array of tokens to free. This should be a NULL-terminated array of strings.
 */
void freeTokens(char** tokens);

/**
 * @brief Removes quotes from a string.
 * 
 * This function removes surrounding quotes from a string, if they are present. If the string is not quoted, it returns the original string.
 * If the string is quoted, it allocates a new string without the quotes and returns it. The original string is freed if quotes are removed.
 * 
 * @param str The string from which quotes should be removed.
 * @return char* A pointer to the new string without quotes. The caller is responsible for freeing this memory.
 */
char* removeQuotes(char* str);

/**
 * @brief Gets the number of tokens in an array.
 * 
 * This function counts the number of tokens in a NULL-terminated array of strings.
 * 
 * @param tokens The array of tokens (NULL-terminated).
 * @return int The number of tokens in the array.
 */
int getTokenCount(char** tokens);

#endif // UTILS_H
