#include "parser.h"
#include "shell_builtins.h"

#include <fcntl.h>
#include <glob.h>

#define COMPARE_TOKEN(token, string) (token && strcmp(token, string) == 0)

/**
 * @brief Parses an array of tokens and generates a command chain.
 * 
 * The function processes tokens to build a command chain. Each command in the chain may consist of multiple simple commands.
 * It handles various operators like pipes, redirections, and chaining operators.
 * 
 * @param tokens Array of tokens to parse.
 * @return CommandChain* Pointer to the generated command chain or NULL on failure.
 */
CommandChain* parseTokens(char** tokens)
{
    // Initialize a new command chain
    CommandChain* chain = initCommandChain();
    if (!chain)
    {
        LOG_DEBUG("Failed to allocate memory for command chain\n");
        return NULL; // Memory allocation failed
    }

    int currentIndexInTokens = 0; // Index to traverse the tokens array

    while (tokens[currentIndexInTokens] != NULL)
    {
        // Initialize a new command
        Command* command = initCommand();
        if (!command)
        {
            LOG_DEBUG("Failed to allocate memory for command\n");
            cleanUpCommandChain(chain);
            return NULL; // Memory allocation failed
        }

        // Initialize a new simple command
        SimpleCommand* simpleCommand = initSimpleCommand();
        if (!simpleCommand)
        {
            LOG_DEBUG("Failed to allocate memory for simple command\n");
            cleanUpCommandChain(chain);
            cleanUpCommand(command);
            return NULL; // Memory allocation failed
        }

        // Process tokens until we encounter a chaining operator or end of tokens
        for (; !IS_NULL(tokens[currentIndexInTokens]) && !IS_CHAINING_OPERATOR(tokens[currentIndexInTokens]); currentIndexInTokens++)
        {
            if (IS_NULL(tokens[currentIndexInTokens]))
            {
                // Push the simpleCommand to the command's simple commands
                if (!simpleCommand->commandName)
                {
                    LOG_DEBUG("Parse error. Null command encountered\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // No command name found
                }
                simpleCommand->execute = getExecutionFunction(simpleCommand->commandName);
                addSimpleCommand(command, simpleCommand);
                simpleCommand = NULL; // No more simple commands
                break;
            }
            else if (IS_PIPE(tokens[currentIndexInTokens]))
            {
                // Handle pipe operator
                if (!simpleCommand->commandName)
                {
                    LOG_DEBUG("Parse error near \'%s\'\n", tokens[currentIndexInTokens]);
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Grammar error: no command before pipe
                }

                if (simpleCommand->outputFD != STDOUT_FD)
                {
                    LOG_DEBUG("Parse error. Cannot pipe to multiple commands\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Error: multiple output redirections
                }

                int pipeFD[2];
                if (pipe(pipeFD) == -1)
                {
                    LOG_DEBUG("Failed to create pipe\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Pipe creation failed
                }

                simpleCommand->outputFD = pipeFD[PIPE_WRITE_END];
                simpleCommand->execute = getExecutionFunction(simpleCommand->commandName);
                addSimpleCommand(command, simpleCommand);

                // Start a new simple command for the next segment
                simpleCommand = initSimpleCommand();
                if (!simpleCommand)
                {
                    LOG_DEBUG("Failed to allocate memory for simple command\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    return NULL; // Memory allocation failed
                }

                // Set the new simple command's inputFD to connect via pipe
                simpleCommand->inputFD = pipeFD[PIPE_READ_END];
            }
            else if (IS_FILE_OUT_REDIR(tokens[currentIndexInTokens]))
            {
                // Handle output redirection
                if (!simpleCommand->commandName)
                {
                    LOG_DEBUG("Parse error. Output redirection encountered before command\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Output redirection without command
                }

                if (simpleCommand->outputFD != STDOUT_FD)
                {
                    LOG_DEBUG("Cannot redirect output to multiple files\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Multiple output redirections
                }

                int fileFD = -1;
                int isAppend = 0;
                if (IS_APPEND(tokens[currentIndexInTokens]))
                    isAppend = 1;

                char* fileNameToken = NULL;
                do {
                    fileNameToken = tokens[++currentIndexInTokens];
                } while (IGNORE(fileNameToken));

                if (isAppend)
                    fileFD = open(fileNameToken, O_WRONLY | O_CREAT | O_APPEND, 0644);
                else
                    fileFD = open(fileNameToken, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fileFD == -1)
                {
                    LOG_DEBUG("Failed to open file for output redirection\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // File open failed
                }

                simpleCommand->outputFD = fileFD;
            }
            else if (IS_FILE_IN_REDIR(tokens[currentIndexInTokens]))
            {
                // Handle input redirection
                if (simpleCommand->inputFD != STDIN_FD)
                {
                    LOG_DEBUG("Cannot redirect input from multiple files\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Multiple input redirections
                }

                char* fileNameToken = NULL;
                do {
                    fileNameToken = tokens[++currentIndexInTokens];
                } while (IGNORE(fileNameToken));

                int fileFD = open(fileNameToken, O_RDONLY);
                if (fileFD == -1)
                {
                    LOG_DEBUG("Failed to open file for input redirection\n");
                    cleanUpCommandChain(chain);
                    return NULL; // File open failed
                }

                simpleCommand->inputFD = fileFD;
            }
            else if (IS_STDERR_REDIR(tokens[currentIndexInTokens]))
            {
                // Handle stderr redirection
                if (simpleCommand->stderrFD != STDERR_FD)
                {
                    LOG_DEBUG("Cannot redirect stderr to multiple files\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Multiple stderr redirections
                }
                
                char* fileNameToken = NULL;
                do {
                    fileNameToken = tokens[++currentIndexInTokens];
                } while (IGNORE(fileNameToken));

                int fileFD = open(fileNameToken, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (fileFD == -1)
                {
                    LOG_DEBUG("Failed to open file for stderr redirection\n");
                    cleanUpCommandChain(chain);
                    return NULL; // File open failed
                }

                simpleCommand->stderrFD = fileFD;
            }
            else if (IGNORE(tokens[currentIndexInTokens]))
            {
                // Ignore irrelevant tokens (e.g., empty tokens)
                continue;
            }
            else if (!simpleCommand->commandName && tokens[currentIndexInTokens][0] == '!' && strlen(tokens[currentIndexInTokens]) > 1)
            {
                // Handle history expansion (!<number> or !<command>)
                if (pushArgs("history", simpleCommand) != 0)
                {
                    LOG_DEBUG("Failed to push argument to simple command\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Argument push failed
                }

                if (pushArgs(tokens[currentIndexInTokens] + 1, simpleCommand) != 0)
                {
                    LOG_DEBUG("Failed to push argument to simple command\n");
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Argument push failed
                }
            }
            else
            {
                // Handle normal tokens: remove quotes and expand wildcards
                tokens[currentIndexInTokens] = removeQuotes(tokens[currentIndexInTokens]);
                glob_t globbuf;
                int globReturn = glob(tokens[currentIndexInTokens], GLOB_NOCHECK | GLOB_TILDE, NULL, &globbuf);

                if (globReturn != 0)
                {
                    LOG_DEBUG("Failed to expand glob\n");
                    globfree(&globbuf);
                    cleanUpCommandChain(chain);
                    cleanUpCommand(command);
                    cleanUpSimpleCommand(simpleCommand);
                    return NULL; // Glob expansion failed
                }

                // Push expanded tokens to the simple command arguments
                for (size_t i = 0; i < globbuf.gl_pathc; i++)
                {
                    if (pushArgs(globbuf.gl_pathv[i], simpleCommand) != 0)
                    {
                        LOG_DEBUG("Failed to push argument to simple command\n");
                        globfree(&globbuf);
                        cleanUpCommandChain(chain);
                        cleanUpCommand(command);
                        cleanUpSimpleCommand(simpleCommand);
                        return NULL; // Argument push failed
                    }
                }

                globfree(&globbuf);
            }
        }
        
        // Push the last simple command to the command's simple commands
        if (simpleCommand && simpleCommand->commandName)
        {
            simpleCommand->execute = getExecutionFunction(simpleCommand->commandName);
            addSimpleCommand(command, simpleCommand);
            simpleCommand = NULL; // No more simple commands
        }

        // Update the chain operator (e.g., ';', '&&', '||')
        command->chainingOperator = COPY(tokens[currentIndexInTokens]);
        if (IS_BACKGROUND(command->chainingOperator))
            command->background = true;

        // Add the command to the chain
        addCommandToChain(chain, command);

        // Move to the next token if it's not NULL
        if (tokens[currentIndexInTokens])
        {
            currentIndexInTokens++;
        }
    }

    return chain; // Return the constructed command chain
}
