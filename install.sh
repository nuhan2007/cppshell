#!/usr/bin/env bash
# Builds mysh and installs it + an app-menu launcher. Leaves the CodeCrafters files untouched.
set -euo pipefail
cd "$(dirname "$0")"

PREFIX="${PREFIX:-$HOME/.local}"
LIBS="${LIBS:-}"   # e.g. LIBS=-lreadline ./install.sh once you add readline

mkdir -p build "$PREFIX/bin"
g++ -std=c++17 -O2 main.cpp -o build/mysh $LIBS
install -m 755 build/mysh "$PREFIX/bin/mysh"
echo "Installed $PREFIX/bin/mysh"

# App-menu entry for Linux desktops (GNOME, KDE, etc.). Harmless on WSL.
APPS="$HOME/.local/share/applications"
mkdir -p "$APPS"
cat > "$APPS/mysh.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=mysh
Comment=A shell written in C++
Exec=$PREFIX/bin/mysh
Terminal=true
Icon=utilities-terminal
Categories=System;
EOF
echo "Added launcher: $APPS/mysh.desktop"

case ":$PATH:" in
  *":$PREFIX/bin:"*) ;;
  *) echo "Note: add $PREFIX/bin to your PATH to run 'mysh' from anywhere." ;;
esac
