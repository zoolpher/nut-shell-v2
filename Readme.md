<p align="center">
  <img src="assets/logo.png" width="300">
</p>

# nut-shell-v2

A Unix shell built from scratch in C++, implementing core shell mechanics: process execution, I/O redirection, pipes, conditional chaining, background jobs, and builtins.

## Features

- Command execution via `fork()`/`execvp()`
- Output redirection (`>`, `>>`) and input redirection (`<`)
- Pipes (`|`) for chaining multiple commands
- Conditional chaining (`&&`) — run next command only if previous succeeded
- Background execution (`&`) with non-blocking job completion reporting
- Builtin commands (`cd`, with `$HOME` fallback)
- Dynamic, colored prompt showing live working directory
- Command-line history and editing via GNU Readline (arrow-key recall)
- Graceful error handling — failed redirections and commands report errors instead of hanging or crashing

## Dependencies

- **Linux or WSL** (this shell relies on POSIX system calls — `fork`, `execvp`, `pipe`, `chdir`, `waitpid` — and won't work on native Windows)
- **g++** with C++17 support
- **GNU Readline development library**

## Setup

1. **Clone the repository**
```bash
   git clone https://github.com/zoolpher/nut-shell.git
   cd nut-shell
```

2. **Install dependencies** (on WSL/Debian/Ubuntu)
```bash
   sudo apt update
   sudo apt install g++ make libreadline-dev
```

3. **Build**
```bash
   make
```

4. **Run**
```bash
   ./nut-shell
```

## Usage examples

```bash
cd some_folder
ls -la > output.txt
cat < names.txt | sort > sorted.txt
sleep 5 & ls -la
ping google.com && echo "connected"
```

Type `exit` to quit.

## Running tests

Two separate test drivers are included for isolated testing of the parser and executor, independent of the main shell.

**Parser tests** — verifies command parsing (redirection, pipes, flags) without executing anything:
```bash
make test_parser
./test_parser
```

**Executor tests** — runs full command execution (fork/exec, pipes, redirection, builtins) through a REPL loop, same as `nut-shell` itself:
```bash
make test_executor
./test_executor
```

Both binaries are excluded from the main `nut-shell` build and only compiled when explicitly requested via `make test_parser` or `make test_executor`.

## Project structure

```
src/            - core implementation (parser, executor, main)
src/helper/     - builtin command handling
include/        - header files
test/           - test drivers for parser and executor
```

## Author

— [@zoolpher](https://github.com/zoolpher)

- B.Tech Computer Science (final year)
- Focus: Systems programming · Operating systems · Process management
- Built nut-shell-v2 (extension of [nut-shell](https://github.com/zoolpher/nut-shell)) as a hands-on exploration of shell internals, process creation, pipes, command parsing, and Unix system calls in C++.

### License

This project is licensed under the [MIT License](LICENSE) - see the LICENSE file for details. You are free to fork, modify, and use this project for your own learning and development!
