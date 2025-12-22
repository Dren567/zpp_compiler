# ZPP Compiler & Interpreter

A complete compiler and interpreter for **ZPP**, a statically-typed language designed for game development and interactive applications.

## Quick Start

```bash
cd compiler
mkdir build && cd build
cmake .. && make -j4
./compiler ../program.zpp
```

## Pipeline

```
Source Code → [Lexer] → [Parser] → [Semantic Analyzer] → [IR Generator] → [Interpreter]
```

## Language Features

| Feature | Support |
|---------|---------|
| **Types** | `int`, `float`, `bool`, `string`, `void` |
| **Control Flow** | `if`/`else`, `while`, `for` |
| **Functions** | User-defined with parameters & return types |
| **Operators** | Arithmetic, comparison, logical |
| **Graphics** | SDL2-based 2D drawing |
| **Input** | Keyboard & text input |

## Building

### Requirements
- C++17 compiler
- CMake 3.10+
- SDL2 & SDL2_image libraries

### Ubuntu/Debian
```bash
sudo apt-get install libsdl2-dev libsdl2-image-dev cmake
cd compiler && mkdir build && cd build && cmake .. && make
```

### macOS
```bash
brew install sdl2 sdl2_image cmake
cd compiler && mkdir build && cd build && cmake .. && make
```

## Examples

### Hello World
```zpp
main() {
    print("Hello, World!\n");
}
```

### Graphics Demo
```zpp
main() {
    screen(800, 600, "Demo");
    clearScreen(255, 255, 255);
    drawRect(100, 100, 200, 150, 0, 0, 255, 1);
    display();
    quit();
}
```

### Game Loop
```zpp
main() {
    screen(1024, 768, "Game");
    let x: int = 512;
    let running: int = 1;
    
    while(running == 1) {
        clearScreen(50, 50, 50);
        drawRect(x, 300, 40, 40, 0, 255, 0, 1);
        
        if(isKeyDown("a") == 1) { x = x - 5; }
        if(isKeyDown("d") == 1) { x = x + 5; }
        
        display();
    }
    quit();
}
```

## API Reference

### I/O Functions
```
print(value, ...)           // Print to console
input(prompt)               // Read text input
key_pressed(prompt)         // Read single keystroke
```

### Graphics Functions
```
screen(width, height, title)                // Create window
clearScreen(r, g, b)                        // Clear to color
drawPixel(x, y, r, g, b)                   // Draw pixel
drawRect(x, y, w, h, r, g, b, filled)      // Draw rectangle
drawLine(x1, y1, x2, y2, r, g, b)          // Draw line
drawCircle(x, y, radius, r, g, b, filled)  // Draw circle
drawTriangle(x1, y1, x2, y2, x3, y3, r, g, b, filled)  // Draw triangle
display()                                   // Update display
quit()                                      // Exit
```

### Input Functions
```
isKeyDown(key_name)         // Check if key held down
```

## Project Structure

```
compiler/
├── include/            # Headers
│   ├── lexer.h        # Tokenization
│   ├── parser.h       # Parsing → AST
│   ├── ir.h           # IR generation → bytecode
│   ├── scematic.h     # Type checking & validation
│   └── graphics.h     # SDL2 graphics engine
├── src/               # Implementation
│   ├── main.cpp       # Interpreter & entry point
│   └── *.cpp
├── test/              # Unit tests
└── CMakeLists.txt     # Build config
```

## Architecture

- **Lexer**: Tokenizes source code
- **Parser**: Recursive descent parsing with operator precedence
- **Semantic Analyzer**: Type checking & symbol table management
- **IR Generator**: Converts AST to bytecode instructions
- **Interpreter**: Stack-based bytecode execution with graphics support

## Features

✅ Static type system with implicit conversions  
✅ 2D drawing primitives (pixels, lines, shapes)  
✅ Sprite/image rendering support  
✅ Keyboard input handling  
✅ Game loop support with real-time graphics  
✅ Comments & escape sequences in strings  
✅ Nested scopes & variable declarations  
✅ Comprehensive error reporting  

## Type System

```cpp
int x = 42;           // Integer
float y = 3.14;       // Float
string name = "Alice"; // String
```

Implicit conversions: `int` ↔ `float`, `int` ↔ `string`

## Error Handling

- **Lexical**: Unterminated strings, unknown tokens
- **Syntax**: Malformed expressions, unexpected tokens
- **Semantic**: Undefined variables, type mismatches
- **Runtime**: Division by zero, invalid conversions

## Limitations

- No arrays yet
- No structs/classes
- No dynamic memory
- Single-threaded
- CPU-based graphics (not GPU-accelerated)

## Future Work

- Array types and indexing
- User-defined types
- Standard library (math, strings)
- Better error messages with location hints
- Performance optimizations

## Testing

```bash
cd build
ctest
```

## License

Open source - Educational and personal use
