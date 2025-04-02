# Modular C Shell

This repository contains a custom UNIX-like command-line shell developed in C. The shell replicates many core functionalities of modern shells (like Bash or Zsh) while emphasizing modular design, extensibility, and robust error handling.

---

## Project Overview

The Modular C Shell is designed to:
- Execute both built-in and external commands.
- Support input/output/error redirection, pipelines, sequential and background execution.
- Expand wildcards in file names.
- Maintain a command history with recall features.
- Handle signals (e.g., SIGINT, SIGTSTP, SIGQUIT) similarly to standard shells.
- Be tested via an automated framework with extensive logging and debugging support (including Valgrind compatibility).

This implementation was developed as an academic project to demonstrate a deep understanding of systems programming, process management, and shell design.

---

## 🔧 Features

- **Custom Shell Prompt:** Dynamic prompt changes via the built-in `prompt` command.
- **Built-in Commands:**
  - `cd` – Change the current directory.
  - `pwd` – Display the current working directory.
  - `exit` – Terminate the shell.
  - `history` – Display the list of previously executed commands.
- **External Command Execution:**
  - Supports pipelines using `|`.
  - Input/output/error redirection with `<`, `>`, and `2>`.
  - Sequential command execution with `;`.
  - Background execution with `&`.
- **Wildcard Expansion:** Automatically expands patterns (e.g., `*.c`) to matching filenames.
- **Command History Recall:**
  - Recall by command number using `!n`.
  - Recall by prefix using `!prefix`.
- **Signal Handling:** Robustly manages signals such as:
  - `CTRL+C` (SIGINT)
  - `CTRL+Z` (SIGTSTP)
  - `CTRL+\` (SIGQUIT)
- **Testing Framework:** An automated test suite verifies shell functionality, including simple and advanced command scenarios.
- **Memory Debugging:** Compatible with Valgrind to assist in detecting memory leaks.

---

## 🛠️ Project Structure

```
modular-c-shell/
├── build/               # Directory for compiled binaries
├── include/             # Header files (.h)
│   ├── command.h        # Definitions for command structures and chain management
│   ├── log.h            # Logging macros and debugging utilities
│   ├── parser.h         # Macros and declarations for command parsing
│   ├── shell_builtins.h # Definitions for built-in command functions
│   └── utils.h          # Utility functions and macros
├── src/                 # C source files (.c)
│   ├── main.c           # Shell entry point and main loop
│   ├── command.c        # Command creation, execution, and cleanup functions
│   ├── parser.c         # Implementation of the command line parser
│   ├── shell_builtins.c # Implementation of built-in shell commands
│   └── utils.c          # Helper functions for string manipulation and logging
├── test/                # Test scripts for verifying shell functionality
│   ├── test_basic.sh    # Basic command and built-in tests
│   └── test_adv.sh      # Advanced tests including pipelines and redirection
├── Makefile             # Build script for compilation and test automation
└── README.md            # This documentation file
```

---

## 🚀 How to Build

To compile the shell, run:
```bash
make
```
This command compiles the source code and creates the executable `Shell` in the `build/` directory.

---

## ▶️ How to Run

To launch the shell, execute:
```bash
./build/Shell
```
The shell supports interactive usage as well as script mode for automated testing.

---

## 🧪 Running Tests

To run the full test suite and verify shell functionality, use:
```bash
make test
```
This executes both basic and advanced test scripts located in the `test/` directory and outputs the results.

---

## 🧹 Clean Build Files

To remove all compiled binaries and object files, run:
```bash
make clean
```

---

## Developer Notes

- **Modular Design:** Core functionalities (command parsing, execution, built-ins, utilities) are separated into distinct modules for improved maintainability and ease of feature addition.
- **Robust Error Handling:** The shell gracefully handles invalid input, file errors, and system interruptions.
- **Debugging Support:** Detailed debug logs are available in debug builds; the project is designed to work seamlessly with Valgrind for memory leak detection.
- **Portability:** The shell is designed to run on most UNIX/Linux systems, with potential minor modifications needed for macOS.

---

## Educational Context

This project demonstrates practical applications of:
- Systems programming in C.
- Process management and inter-process communication.
- Shell command parsing and execution.
- Modular software design and debugging techniques.

---

## License

This project is for educational purposes only. For reuse or distribution, please contact the author for permission.

---

*For further details on design decisions, source code implementations, test evidence, and evaluation, please refer to the full doc file*
