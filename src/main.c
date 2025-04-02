#include "utils.h"
#include "command.h"
#include "parser.h"
#include "shell_builtins.h"

#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

// Global variables
int lastExitStatus = 0;  ///< Stores the exit status of the last executed command

ShellState* globalShellState; ///< Holds the state of the shell, including history and prompt settings

FILE* scriptFile; ///< File pointer for reading script files in non-interactive mode

/**
 * @brief Reads input from either the terminal or a script file.
 * 
 * @param interactive Indicates if the shell is running in interactive mode.
 * @return char* Pointer to the input string or NULL on failure or end-of-file.
 */
char* getInput(int interactive)
{
    char* input = malloc(MAX_STRING_LENGTH);  ///< Allocate memory for input buffer
    if (input == NULL) {
        LOG_ERROR("Memory allocation failed");
        exit(EXIT_FAILURE);  ///< Exit if memory allocation fails
    }

    if (interactive)
    {
        int again = 1;
        char *linept;  ///< Pointer to the line buffer

        while (again) {
            again = 0;
            printf("%s ", globalShellState->prompt_buffer);  ///< Print prompt
            linept = fgets(input, MAX_STRING_LENGTH, stdin);  ///< Read input from stdin
            if (linept == NULL) {
                if (feof(stdin)) {
                    free(input);
                    return NULL;  ///< End of file (Ctrl-D)
                } else if (errno == EINTR) {
                    again = 1;  ///< Signal interruption, read again
                } else {
                    LOG_ERROR("Error reading input: %s\n", strerror(errno));
                    free(input);
                    exit(EXIT_FAILURE);  ///< Exit on read error
                }
            }
        }

        // Remove the trailing newline character from input
        size_t ln = strlen(input);
        if (ln > 0 && input[ln - 1] == '\n') {
            input[ln - 1] = '\0';
        }
    }
    else
    {
        size_t len = 0;
        ssize_t read = getline(&input, &len, scriptFile);  ///< Read input from script file
        if (read == -1) {
            free(input);
            return NULL;  ///< End of file (script ended)
        }

        // Remove trailing newline character
        if (read > 0 && input[read - 1] == '\n') {
            input[read - 1] = '\0';
        }
    }

    return input;
}

/**
 * @brief Handles SIGINT signal (Ctrl-C).
 * 
 * @param signo Signal number.
 */
void sigint_handler(int signo) {
    LOG_DEBUG("\nCTRL-C pressed. signo: %d\n", signo);  ///< Log SIGINT signal
}

/**
 * @brief Handles SIGTSTP signal (Ctrl-Z).
 * 
 * @param signo Signal number.
 */
void sigtstp_handler(int signo) {
    LOG_DEBUG("\nCTRL-Z pressed. signo: %d\n", signo);  ///< Log SIGTSTP signal
}

/**
 * @brief Handles SIGQUIT signal (Ctrl-\).
 * 
 * @param signo Signal number.
 */
void sigquit_handler(int signo) {
    LOG_DEBUG("\nCTRL-\\ pressed. signo: %d\n", signo);  ///< Log SIGQUIT signal
}

/**
 * @brief Handles SIGCHLD signal, reaping child processes.
 * 
 * @param signo Signal number.
 */
void sigchld_handler(int signo) {
    (void) signo;  ///< Unused parameter
    int more = 1;  ///< Flag to check if more zombies need to be reaped
    pid_t pid;  ///< PID of the zombie process
    int status;  ///< Termination status of the zombie process

    // Reap all zombie processes
    while (more) {
        pid = waitpid(-1, &status, WNOHANG);  ///< Non-blocking wait for child processes
        if (pid <= 0) {
            more = 0;  ///< No more zombies or error
        }
    }
}

/**
 * @brief Main function of the shell.
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit status of the shell
 */
int main(int argc, char** argv)
{
    // Default to interactive mode
    int interactive = 1;
    scriptFile = NULL;

    // Check for correct number of arguments
    if (argc > 2)
    {
        LOG_ERROR("Usage: %s [script]\n", argv[0]);
        exit(1);  ///< Exit if arguments are incorrect
    }

    // If a script is provided, open it and set non-interactive mode
    if (argc == 2)
    {
        interactive = 0;
        LOG_DEBUG("Running script %s\n", argv[1]);
        scriptFile = fopen(argv[1], "r");
        if (!scriptFile)
        {
            LOG_ERROR("Error opening script %s: %s\n", argv[1], strerror(errno));
            exit(1);  ///< Exit if script file cannot be opened
        }
    }

    // Initialize global shell state
    globalShellState = init_shell_state();

    LOG_DEBUG("Starting shell\n");

    char delimiter = ' ';  ///< Tokenization delimiter

    // Set up signal handlers
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        LOG_ERROR("Unable to register SIGINT handler");
        exit(EXIT_FAILURE);  ///< Exit if SIGINT handler cannot be set
    }

    if (signal(SIGTSTP, sigtstp_handler) == SIG_ERR) {
        LOG_ERROR("Unable to register SIGTSTP handler");
        exit(EXIT_FAILURE);  ///< Exit if SIGTSTP handler cannot be set
    }

    if (signal(SIGQUIT, sigquit_handler) == SIG_ERR) {
        LOG_ERROR("Unable to register SIGQUIT handler");
        exit(EXIT_FAILURE);  ///< Exit if SIGQUIT handler cannot be set
    }

    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        LOG_ERROR("Unable to register SIGCHLD handler");
        exit(EXIT_FAILURE);  ///< Exit if SIGCHLD handler cannot be set
    }

    while (1)
    {
        char* input = getInput(interactive);

        // Handle end-of-file (Ctrl-D) or errors
        if (input == NULL)
        {
            if (feof(stdin)) {
                printf("\nEOF detected. Exiting shell.\n");
            } else {
                LOG_ERROR("Error reading input: %s\n", strerror(errno));
            }
            break;  ///< Exit the shell loop
        }

        // Skip empty input
        if (strcmp(input, "") == 0)
        {
            free(input);
            continue;
        }

        // Exit the shell if "exit" command is entered
        if (strcmp(input, "exit") == 0)
        {
            free(input);
            break;  ///< Exit the shell loop
        }

        // Add input to command history
        add_to_history(&globalShellState->history, input);

        // Tokenize the input string
        char** tokens = tokenizeString(input, delimiter);

        // Log each token for debugging
        for (int i = 0; tokens[i] != NULL; i++) {
            LOG_DEBUG("Token %d: [%s]\n", i, tokens[i]);
        }

        // Parse tokens into a CommandChain
        CommandChain* commandChain = parseTokens(tokens);

        // Display the command chain for debugging
        printCommandChain(commandChain);
        
        // Execute the command chain and get the exit status
        int status = executeCommandChain(commandChain);
        LOG_DEBUG("Command executed with status %d\n", status);

        // Free memory allocated for tokens
        freeTokens(tokens);

        // Free memory allocated for command chain
        cleanUpCommandChain(commandChain);

        // Free memory allocated for input buffer
        free(input);
    }

    // Clean up command history
    clean_history(&globalShellState->history);

    return 0;  ///< Return success status
}
