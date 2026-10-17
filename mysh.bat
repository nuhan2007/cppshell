@echo off
rem Double-click to open mysh in its own window (requires WSL + running install.sh inside WSL first).
title mysh
wsl.exe --cd ~ -e sh -c "exec $HOME/.local/bin/mysh"
