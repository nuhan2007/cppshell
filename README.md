# mysh

A small Unix shell written in C++17. Supports builtins (`cd`, `pwd`, `echo`, `type`, `exit`), running programs from `PATH`, quoting, and output redirection (`>`, `>>`, `2>`, `2>>`).

## Install

Requires Linux, macOS, or WSL on Windows, plus `g++` with C++17 support (`sudo apt install g++` on Ubuntu/WSL).

```bash
git clone https://github.com/nuhan2007/cppshell.git
cd cppshell
./install.sh
```

This installs `mysh` to `~/.local/bin` and adds a "mysh" entry to your Linux app menu.

## Run

- **Terminal:** `mysh`
- **Windows (WSL):** after running `install.sh` inside WSL, double-click `mysh.bat` (or make a shortcut to it and pin it to Start/taskbar).
