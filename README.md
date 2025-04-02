# Modular C Shell

This project is a custom UNIX-like shell written in C  It replicates core features of common shells like Bash and Zsh, with added support for modularity, extensibility, and testing.

## 🧰 Features

- 🔹 Customizable shell prompt (`prompt`)
- 🔹 Built-in commands: `cd`, `pwd`, `exit`, `history`
- 🔹 I/O Redirection: `>`, `<`, `2>`
- 🔹 Wildcard Expansion (e.g., `*.c`)
- 🔹 Pipelining (e.g., `ls | grep txt`)
- 🔹 Sequential (`;`) and Background (`&`) execution
- 🔹 Command history with `!n` and prefix search
- 🔹 Signal handling (CTRL-C, CTRL-Z, CTRL-\)
- 🔹 Memory debugging with Valgrind
- 🔹 Fully scriptable with test automation

## 🏗️ Build & Run

```bash
# Clone the repository
git clone https://github.com/aswinvmohanyeroor/modular-c-shell.git
cd modular-c-shell

# Build
make

# Run
./build/Shell
