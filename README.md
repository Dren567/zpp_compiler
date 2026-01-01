# ZPP Compiler & Interpreter

A complete compiler and interpreter for **ZPP**, a statically-typed language designed for game development and interactive applications.

---

## One-Command Quick Start

Clone the repository, build the compiler, and install `zpp` globally:

```bash
git clone https://github.com/Dren567/zpp_compiler.git ~/zpp_compiler && \
cd ~/zpp_compiler && \
chmod +x install.sh && \
./install.sh && \
echo "ZPP installed! You can now run:" && \
echo "zpp path/to/file.zpp"
```

Example:

```bash
cd ~/Desktop
zpp zpp_compiler/examples/main.zpp
```

No matter where your `.zpp` file is located, `zpp` will automatically find the compiler and run it.

---

## Pipeline

```
Source Code → [Lexer] → [Parser] → [Semantic Analyzer] → [IR Generator] → [Interpreter]
```

---

## Language Features

| Feature | Support |
|---------|---------|
| **Types** | `int`, `float`, `bool`, `string`, `void` |
| **Control Flow** | `if`/`else`, `while`, `for` |
| **Functions** | User-defined with parameters & return types |
| **Operators** | Arithmetic, comparison, logical |
| **Graphics** | SDL2-based 2D drawing |
| **Input** | Keyboard & text input |

---

## Building Requirements

- C++17 compiler  
- CMake 3.10+  
- SDL2 & SDL2_image libraries  

### Linux (Debian/Ubuntu)
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev cmake build-essential
```

### Arch / CachyOS
```bash
sudo pacman -Syu --needed cmake make gcc sdl2 sdl2_image
```

### Windows (Git Bash / MSYS2 / WSL)
Ensure CMake and Make are installed in your shell environment.  
After installing dependencies, run `install.sh` (handled in the one-command setup).

---

## Variable Declaration

All variables are declared using the syntax:

```
let:name type = value
```

### Examples
```zpp
let:x int = 42
let:y float = 3.14
let:name string = "Alice"
let:flag bool = true
```

Implicit conversions: `int` ↔ `float`, `int` ↔ `string`

---

## Examples

### Hello World
```zpp
main() {
    let:greeting string = "Hello, World!\n"
    print(greeting)
}
```

### Graphics Demo
```zpp
main() {
    screen(800, 600, "Demo")
    clearScreen(255, 255, 255)
    drawRect(100, 100, 200, 150, 0, 0, 255, 1)
    display()
    quit()
}
```

### Game Loop
```zpp
main() {
    screen(1024, 768, "Game")
    let:x int = 512
    let:running int = 1
    
    while(running == 1) {
        clearScreen(50, 50, 50)
        drawRect(x, 300, 40, 40, 0, 255, 0, 1)
        
        if(isKeyDown("a") == 1) { x = x - 5 }
        if(isKeyDown("d") == 1) { x = x + 5 }
        
        display()
    }
    quit()
}
```

---

## API Reference

### I/O Functions
```
print(value, ...)
input(prompt)
key_pressed(prompt)
```

### Graphics Functions
```
screen(width, height, title)
clearScreen(r, g, b)
drawPixel(x, y, r, g, b)
drawRect(x, y, w, h, r, g, b, filled)
drawLine(x1, y1, x2, y2, r, g, b)
drawCircle(x, y, radius, r, g, b, filled)
drawTriangle(x1, y1, x2, y2, x3, y3, r, g, b, filled)
display()
quit()
```

### Input Functions
```
isKeyDown(key_name)
```

---

## Project Structure

```
compiler/
├── include/
│   ├── lexer.h
│   ├── parser.h
│   ├── ir.h
│   ├── scematic.h
│   └── graphics.h
├── src/
│   ├── main.cpp
│   └── *.cpp
├── test/
├── build/
└── CMakeLists.txt
```

`zpp` is installed in `~/.local/bin` (Linux) or `$HOME/bin` (Windows) and acts as the global command to run programs.

---

## Features

✅ Static type system with implicit conversions  
✅ 2D drawing primitives (pixels, lines, shapes)  
✅ Sprite/image rendering support  
✅ Keyboard input handling  
✅ Game loop support  
✅ Comments & escape sequences in strings  
✅ Nested scopes & variable declarations  
✅ Comprehensive error reporting  

---

## Error Handling

- Lexical: Unterminated strings, unknown tokens  
- Syntax: Malformed expressions, unexpected tokens  
- Semantic: Undefined variables, type mismatches  
- Runtime: Division by zero, invalid conversions  

---

## Limitations

- No arrays yet  
- No structs/classes  
- No dynamic memory  
- Single-threaded  
- CPU-based graphics  

---

## Future Work

- Array types and indexing  
- User-defined types  
- Standard library (math, strings)  
- Better error messages with location hints  
- Performance optimizations  

---

## Testing

```bash
ctest
```

---

## License

Open source - Educational and personal use
