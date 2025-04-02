/**
 * @file parser.h
 * @brief Contains macros and the definition for the main parser function that processes command tokens into a command chain.
 * @version 0.1
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef PARSER_H
#define PARSER_H

#include "command.h"

// Useful macros for readability

/**
 * @brief Checks if the given token is a chaining operator.
 * 
 * Chaining operators are used to separate commands in a command chain, such as `&` and `;`. This macro returns 1 if the token matches one of these operators, otherwise 0.
 * 
 * @param token The token to check
 * @return int 1 if the token is a chaining operator, 0 otherwise
 */
#define IS_CHAINING_OPERATOR(token) (strcmp(token, "&") == 0 || strcmp(token, ";") == 0)

/**
 * @brief Checks if the given token is a background operator.
 * 
 * The background operator is `&`, which indicates that the command should be executed in the background. This macro returns 1 if the token matches `&`, otherwise 0.
 * 
 * @param token The token to check
 * @return int 1 if the token is a background operator, 0 otherwise
 */
#define IS_BACKGROUND(token) (token && strcmp(token, "&") == 0)

/**
 * @brief Checks if the given token is a pipe operator.
 * 
 * The pipe operator `|` is used to pass the output of one command as input to the next command. This macro returns 1 if the token matches `|`, otherwise 0.
 * 
 * @param token The token to check
 * @return int 1 if the token is a pipe, 0 otherwise
 */
#define IS_PIPE(token) (strcmp(token, "|") == 0)

/**
 * @brief Checks if the given token is a file output redirection operator.
 * 
 * File output redirection operators are `>` (overwrite) and `>>` (append). This macro returns 1 if the token matches one of these operators, otherwise 0.
 * 
 * @param token The token to check
 * @return int 1 if the token is a file output redirection operator, 0 otherwise
 */
#define IS_FILE_OUT_REDIR(token) (strcmp(token, ">") == 0 || strcmp(token, ">>") == 0)

/**
 * @brief Checks if the given token is a file input redirection operator.
 * 
 * The file input redirection operator is `<`. This macro returns 1 if the token matches `<`, otherwise 0.
 * 
 * @param token The token to check
 * @return int 1 if the token is a file input redirection operator, 0 otherwise
 */
#define IS_FILE_IN_REDIR(token) (strcmp(token, "<") == 0) 

/**
 * @brief Checks if the given token is a standard error redirection operator.
 * 
 * The standard error redirection operator is `2>`. This macro returns 1 if the token matches `2>`, otherwise 0.
 * 
 * @param token The token to check
 * @return int 1 if the token is a standard error redirection operator, 0 otherwise
 */
#define IS_STDERR_REDIR(token) (strcmp(token, "2>") == 0) 

/**
 * @brief Checks if the given token is NULL.
 * 
 * This macro returns 1 if the token is NULL, which typically indicates an invalid or uninitialized token.
 * 
 * @param token The token to check
 * @return int 1 if the token is NULL, 0 otherwise
 */
#define IS_NULL(token) (!token)

/**
 * @brief Checks if the given token is ignorable.
 * 
 * Ignorable tokens include spaces, tabs, newlines, and empty strings. This macro returns 1 if the token is one of these ignorable values, otherwise 0.
 * 
 * @param token The token to check
 * @return int 1 if the token is ignorable, 0 otherwise
 */
#define IGNORE(token) (token && (strcmp(token, " ") == 0 || strcmp(token, "\t") == 0 || strcmp(token, "\n") == 0 || strcmp(token, "") == 0))

/**
 * @brief Checks if the given token is an append operator.
 * 
 * The append operator is `>>`, used for appending output to a file. This macro returns 1 if the token matches `>>`, otherwise 0.
 * 
 * @param token The token to check
 * @return int 1 if the token is an append operator, 0 otherwise
 */
#define IS_APPEND(token) (strcmp(token, ">>") == 0)

/**
 * @brief Parses an array of tokens into a command chain.
 * 
 * The `parseTokens` function takes an array of tokens (strings) and processes them into a structured command chain. The tokens array is assumed to be null-terminated. It is the caller's responsibility to free the allocated memory for the returned CommandChain.
 * 
 * @param tokens The array of tokens to parse. Should be null-terminated.
 * @return CommandChain* Pointer to the constructed CommandChain. Returns NULL if parsing fails.
 */
CommandChain* parseTokens(char** tokens);

#endif // PARSER_H
