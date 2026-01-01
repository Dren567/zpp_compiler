#!/bin/bash
set -e

OS=""
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
elif [ "$OS" = "" ]; then
    case "$(uname -s)" in
        MINGW*|MSYS*|CYGWIN*|Linux)
            OS="windows"
            ;;
    esac
fi

echo "Detected OS: $OS"

install_deps() {
    case "$OS" in
        arch|manjaro|cachy)
            sudo pacman -Syu --needed cmake make gcc
            ;;
        ubuntu|debian)
            sudo apt update
            sudo apt install -y cmake build-essential
            ;;
        fedora)
            sudo dnf install -y cmake make gcc
            ;;
        windows)
            echo "Make sure you have CMake and Make installed in Git Bash/MSYS2 or WSL."
            ;;
        *)
            echo "Unsupported OS. Please install cmake and make manually."
            ;;
    esac
}

read -p "Do you want to install required dependencies (cmake, make, gcc)? [Y/n]: " answer
answer=${answer:-Y}
if [[ $answer =~ ^[Yy]$ ]]; then
    install_deps
fi

BUILD_DIR="$(pwd)/compiler/build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake ..
make
cd ../../..

if [[ "$OS" == "windows" ]]; then
    BIN_DIR="$HOME/bin"
else
    BIN_DIR="$HOME/.local/bin"
fi

mkdir -p "$BIN_DIR"
ZPP_WRAPPER="$BIN_DIR/zpp"

cat > "$ZPP_WRAPPER" << 'EOF'
#!/bin/bash
COMPILER="$HOME/Desktop/zpp_compiler/compiler/build/compiler"
if [ ! -x "$COMPILER" ]; then
    echo "Compiler not found. Build failed?"
    exit 1
fi
if [ $# -eq 0 ]; then
    echo "Usage: zpp <file.zpp>"
    exit 1
fi
exec "$COMPILER" "$@"
EOF

chmod +x "$ZPP_WRAPPER"
echo "ZPP installed successfully!"
echo "You can now run: zpp <file.zpp> from anywhere."
if [[ "$OS" == "windows" ]]; then
    echo "Make sure $BIN_DIR is added to your PATH in Git Bash or WSL."
fi
