/**
 * @file utils.c
 * @brief Function definitions for the utility functions.
 * @version 0.1
 *
 */

#include "utils.h"

#include <string.h>
#include <stdlib.h>

/**
 * @brief Tokenizes the input string based on a specified delimiter.
 * 
 * Splits the input string into an array of tokens using the provided delimiter. Handles quoted substrings
 * so that delimiters inside quotes are not considered as token separators.
 * 
 * @param input The string to tokenize.
 * @param delimiter The character used to split the input string into tokens.
 * @return char** An array of tokens, with the last element set to NULL. Returns NULL if memory allocation fails.
 */
char **tokenizeString(const char *input, char delimiter)
{
    int input_length = strlen(input);
    char **tokens = (char **)malloc(sizeof(char *) * input_length);
    int token_count = 0;

    int i = 0;
    int token_start = 0;
    int inside_quotes = 0;

    while (input[i] != '\0')
    {
        if (input[i] == delimiter && !inside_quotes)
        {
            int token_length = i - token_start;
            tokens[token_count] = (char *)malloc(sizeof(char) * (token_length + 1));
            strncpy(tokens[token_count], input + token_start, token_length);
            tokens[token_count][token_length] = '\0';
            token_count++;
            token_start = i + 1;
        }
        else if (input[i] == '"' || input[i] == '\'')
        {
            inside_quotes = !inside_quotes;
        }
        i++;
    }

    int token_length = i - token_start;
    tokens[token_count] = (char *)malloc(sizeof(char) * (token_length + 1));
    strncpy(tokens[token_count], input + token_start, token_length);
    tokens[token_count][token_length] = '\0';
    token_count++;

    char** temp = (char **)realloc(tokens, sizeof(char *) * (token_count + 1));
    if (!temp)
        return NULL;
    
    tokens = temp;
    temp = NULL;
    
    tokens[token_count] = NULL;

    return tokens;
}

/**
 * @brief Counts the number of tokens in an array of tokens.
 * 
 * Iterates through the token array and counts the number of non-NULL elements.
 * 
 * @param tokens The array of tokens.
 * @return int The number of tokens in the array.
 */
int getTokenCount(char **tokens)
{
    int token_count = 0;
    while (tokens[token_count] != NULL)
    {
        token_count++;
    }
    return token_count;
}

/**
 * @brief Frees the memory allocated for an array of tokens.
 * 
 * Deallocates memory for each token in the array and then frees the array itself.
 * 
 * @param tokens The array of tokens to free.
 */
void freeTokens(char **tokens)
{
    for (int i = 0; tokens[i] != NULL; i++)
    {
        free(tokens[i]);
    }
    free(tokens);
}

/**
 * @brief Removes surrounding quotes from a string.
 * 
 * If the input string is enclosed in quotes (single or double), creates a new string without the quotes and
 * returns it. Otherwise, returns the original string.
 * 
 * @param inputString The string to process.
 * @return char* The modified string without quotes, or the original string if not enclosed in quotes.
 */
char *removeQuotes(char *inputString)
{
    int inputLength = strlen(inputString);

    // Check if the string is long enough to contain quotes
    if (inputLength < 2)
    {
        // String is too short to be enclosed in quotes
        return inputString;
    }

    // Check if the string is enclosed in quotes
    if ((inputString[0] == '"' && inputString[inputLength - 1] == '"') || (inputString[0] == '\'' && inputString[inputLength - 1] == '\''))
    {
        // Create a modified string without the quotes
        size_t modifiedLength = inputLength - 2;
        char *modifiedString = malloc((modifiedLength + 1) * sizeof(char));
        strncpy(modifiedString, inputString + 1, modifiedLength);
        modifiedString[modifiedLength] = '\0';
        free(inputString);
        return modifiedString;
    }
    else
    {
        // String is not enclosed in quotes
        return inputString; // Return a copy of the input string
    }
}
