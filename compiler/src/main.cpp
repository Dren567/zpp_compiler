#include <iostream>
#include <fstream>
#include <sstream>
#include <termios.h>
#include <unistd.h>

#include "lexer.h"
#include "parser.h"
#include "ir.h"
#include "graphics.h"

// readFile: Read entire file contents into a string
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        exit(1);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// readSingleKey: Read a single character from stdin without waiting for Enter
// Uses termios to disable canonical mode (line buffering)
char readSingleKey() {
    struct termios old_settings, new_settings;
    tcgetattr(STDIN_FILENO, &old_settings);
    new_settings = old_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);    // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
    
    char ch;
    read(STDIN_FILENO, &ch, 1);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);  // Restore original settings
    return ch;
}

// Global graphics instance - shared across the interpreter
Graphics* g_graphics = nullptr;

// Value type: Variant that can hold int, double, string, or bool
// Used for storing runtime values during IR interpretation
#include <map>
#include <variant>
using Value = std::variant<int, double, std::string, bool>;

// interpretIR: Execute the IR bytecode
// Walks through the IR instructions and executes them
void interpretIR(const IRProgram& ir) {
    for (const auto& func : ir.functions) {
        if (func.name == "main") {  // Only execute main function
            std::map<std::string, Value> temps;           // Runtime value storage
            std::map<std::string, size_t> labels;         // Label address lookup
            
            // First pass: find all labels for jump target resolution
            for (size_t i = 0; i < func.instructions.size(); ++i) {
                if (func.instructions[i].opcode == IROpCode::LABEL) {
                    labels[func.instructions[i].label] = i;
                }
            }
            
            // Debug: print IR instructions
            // std::cerr << "=== IR Instructions ===" << std::endl;
            // for (size_t i = 0; i < func.instructions.size(); ++i) {
            //     std::cerr << i << ": " << func.instructions[i].toString() << std::endl;
            // }
            
            size_t ip = 0;
            while (ip < func.instructions.size()) {
                const auto& instr = func.instructions[ip];
                
                if (instr.opcode == IROpCode::LOAD_INT) {
                    int v = std::stoi(instr.operands[0].name);
                    temps[instr.result.toString()] = v;
                } else if (instr.opcode == IROpCode::LOAD_FLOAT) {
                    double v = std::stod(instr.operands[0].name);
                    temps[instr.result.toString()] = v;
                } else if (instr.opcode == IROpCode::LOAD_STRING) {
                    temps[instr.result.toString()] = instr.operands[0].name;
                } else if (instr.opcode == IROpCode::ADD) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    
                    // Helper to convert Value to int
                    auto toInt = [](const Value& v) -> int {
                        if (std::holds_alternative<int>(v)) return std::get<int>(v);
                        if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                        if (std::holds_alternative<std::string>(v)) return std::stoi(std::get<std::string>(v));
                        throw std::runtime_error("Cannot convert to int");
                    };
                    
                    temps[instr.result.toString()] = toInt(a) + toInt(b);
                } else if (instr.opcode == IROpCode::SUB) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    
                    auto toInt = [](const Value& v) -> int {
                        if (std::holds_alternative<int>(v)) return std::get<int>(v);
                        if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                        if (std::holds_alternative<std::string>(v)) return std::stoi(std::get<std::string>(v));
                        throw std::runtime_error("Cannot convert to int");
                    };
                    
                    temps[instr.result.toString()] = toInt(a) - toInt(b);
                } else if (instr.opcode == IROpCode::MUL) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    
                    auto toInt = [](const Value& v) -> int {
                        if (std::holds_alternative<int>(v)) return std::get<int>(v);
                        if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                        if (std::holds_alternative<std::string>(v)) return std::stoi(std::get<std::string>(v));
                        throw std::runtime_error("Cannot convert to int");
                    };
                    
                    temps[instr.result.toString()] = toInt(a) * toInt(b);
                } else if (instr.opcode == IROpCode::DIV) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    
                    auto toInt = [](const Value& v) -> int {
                        if (std::holds_alternative<int>(v)) return std::get<int>(v);
                        if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                        if (std::holds_alternative<std::string>(v)) return std::stoi(std::get<std::string>(v));
                        throw std::runtime_error("Cannot convert to int");
                    };
                    
                    int divisor = toInt(b);
                    if (divisor == 0) throw std::runtime_error("Division by zero");
                    temps[instr.result.toString()] = toInt(a) / divisor;
                } else if (instr.opcode == IROpCode::MOD) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    
                    auto toInt = [](const Value& v) -> int {
                        if (std::holds_alternative<int>(v)) return std::get<int>(v);
                        if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                        if (std::holds_alternative<std::string>(v)) return std::stoi(std::get<std::string>(v));
                        throw std::runtime_error("Cannot convert to int");
                    };
                    
                    int divisor = toInt(b);
                    if (divisor == 0) throw std::runtime_error("Division by zero");
                    temps[instr.result.toString()] = toInt(a) % divisor;
                } else if (instr.opcode == IROpCode::CONCAT) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    std::string result = std::visit([](auto x) -> std::string {
                        if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int>) {
                            return std::to_string(x);
                        } else if constexpr (std::is_same_v<std::decay_t<decltype(x)>, double>) {
                            return std::to_string(x);
                        } else if constexpr (std::is_same_v<std::decay_t<decltype(x)>, bool>) {
                            return x ? "true" : "false";
                        } else {
                            return x;
                        }
                    }, a) + std::visit([](auto x) -> std::string {
                        if constexpr (std::is_same_v<std::decay_t<decltype(x)>, int>) {
                            return std::to_string(x);
                        } else if constexpr (std::is_same_v<std::decay_t<decltype(x)>, double>) {
                            return std::to_string(x);
                        } else if constexpr (std::is_same_v<std::decay_t<decltype(x)>, bool>) {
                            return x ? "true" : "false";
                        } else {
                            return x;
                        }
                    }, b);
                    temps[instr.result.toString()] = result;
                } else if (instr.opcode == IROpCode::PRINT) {
                    auto v = temps[instr.operands[0].toString()];
                    if (std::holds_alternative<int>(v)) std::cout << std::get<int>(v);
                    else if (std::holds_alternative<double>(v)) std::cout << std::get<double>(v);
                    else if (std::holds_alternative<bool>(v)) std::cout << (std::get<bool>(v) ? "true" : "false");
                    else if (std::holds_alternative<std::string>(v)) std::cout << std::get<std::string>(v);
                    std::cout.flush();
                } else if (instr.opcode == IROpCode::LT) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    int result = std::visit([](auto x, auto y) -> int {
                        using X = std::decay_t<decltype(x)>;
                        using Y = std::decay_t<decltype(y)>;
                        if constexpr (std::is_same_v<X, int> && std::is_same_v<Y, int>) {
                            return x < y ? 1 : 0;
                        } else if constexpr ((std::is_same_v<X, int> && std::is_same_v<Y, double>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, int>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, double>)) {
                            return static_cast<double>(x) < static_cast<double>(y) ? 1 : 0;
                        } else if constexpr (std::is_same_v<X, std::string> && std::is_same_v<Y, std::string>) {
                            return x < y ? 1 : 0;
                        } else {
                            throw std::runtime_error("Invalid types for LT");
                        }
                    }, a, b);
                    temps[instr.result.toString()] = result;
                } else if (instr.opcode == IROpCode::GT) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    int result = std::visit([](auto x, auto y) -> int {
                        using X = std::decay_t<decltype(x)>;
                        using Y = std::decay_t<decltype(y)>;
                        if constexpr (std::is_same_v<X, int> && std::is_same_v<Y, int>) {
                            return x > y ? 1 : 0;
                        } else if constexpr ((std::is_same_v<X, int> && std::is_same_v<Y, double>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, int>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, double>)) {
                            return static_cast<double>(x) > static_cast<double>(y) ? 1 : 0;
                        } else if constexpr (std::is_same_v<X, std::string> && std::is_same_v<Y, std::string>) {
                            return x > y ? 1 : 0;
                        } else {
                            throw std::runtime_error("Invalid types for GT");
                        }
                    }, a, b);
                    temps[instr.result.toString()] = result;
                } else if (instr.opcode == IROpCode::LE) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    int result = std::visit([](auto x, auto y) -> int {
                        using X = std::decay_t<decltype(x)>;
                        using Y = std::decay_t<decltype(y)>;
                        if constexpr (std::is_same_v<X, int> && std::is_same_v<Y, int>) {
                            return x <= y ? 1 : 0;
                        } else if constexpr ((std::is_same_v<X, int> && std::is_same_v<Y, double>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, int>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, double>)) {
                            return static_cast<double>(x) <= static_cast<double>(y) ? 1 : 0;
                        } else if constexpr (std::is_same_v<X, std::string> && std::is_same_v<Y, std::string>) {
                            return x <= y ? 1 : 0;
                        } else {
                            throw std::runtime_error("Invalid types for LE");
                        }
                    }, a, b);
                    temps[instr.result.toString()] = result;
                } else if (instr.opcode == IROpCode::GE) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    int result = std::visit([](auto x, auto y) -> int {
                        using X = std::decay_t<decltype(x)>;
                        using Y = std::decay_t<decltype(y)>;
                        if constexpr (std::is_same_v<X, int> && std::is_same_v<Y, int>) {
                            return x >= y ? 1 : 0;
                        } else if constexpr ((std::is_same_v<X, int> && std::is_same_v<Y, double>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, int>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, double>)) {
                            return static_cast<double>(x) >= static_cast<double>(y) ? 1 : 0;
                        } else if constexpr (std::is_same_v<X, std::string> && std::is_same_v<Y, std::string>) {
                            return x >= y ? 1 : 0;
                        } else {
                            throw std::runtime_error("Invalid types for GE");
                        }
                    }, a, b);
                    temps[instr.result.toString()] = result;
                } else if (instr.opcode == IROpCode::EQ) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    int result = std::visit([](auto x, auto y) -> int {
                        using X = std::decay_t<decltype(x)>;
                        using Y = std::decay_t<decltype(y)>;
                        if constexpr (std::is_same_v<X, int> && std::is_same_v<Y, int>) {
                            return x == y ? 1 : 0;
                        } else if constexpr ((std::is_same_v<X, int> && std::is_same_v<Y, double>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, int>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, double>)) {
                            return static_cast<double>(x) == static_cast<double>(y) ? 1 : 0;
                        } else if constexpr (std::is_same_v<X, std::string> && std::is_same_v<Y, std::string>) {
                            return x == y ? 1 : 0;
                        } else {
                            throw std::runtime_error("Invalid types for EQ");
                        }
                    }, a, b);
                    temps[instr.result.toString()] = result;
                } else if (instr.opcode == IROpCode::NE) {
                    auto a = temps[instr.operands[0].toString()];
                    auto b = temps[instr.operands[1].toString()];
                    int result = std::visit([](auto x, auto y) -> int {
                        using X = std::decay_t<decltype(x)>;
                        using Y = std::decay_t<decltype(y)>;
                        if constexpr (std::is_same_v<X, int> && std::is_same_v<Y, int>) {
                            return x != y ? 1 : 0;
                        } else if constexpr ((std::is_same_v<X, int> && std::is_same_v<Y, double>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, int>) ||
                                             (std::is_same_v<X, double> && std::is_same_v<Y, double>)) {
                            return static_cast<double>(x) != static_cast<double>(y) ? 1 : 0;
                        } else if constexpr (std::is_same_v<X, std::string> && std::is_same_v<Y, std::string>) {
                            return x != y ? 1 : 0;
                        } else {
                            throw std::runtime_error("Invalid types for NE");
                        }
                    }, a, b);
                    temps[instr.result.toString()] = result;
                } else if (instr.opcode == IROpCode::JZ) {
                    int cond = std::get<int>(temps[instr.operands[0].toString()]);
                    if (cond == 0) {
                        ip = labels[instr.label];
                        continue;
                    }
                } else if (instr.opcode == IROpCode::JMP) {
                    ip = labels[instr.label];
                    continue;
                } else if (instr.opcode == IROpCode::STORE) {
                    temps[instr.result.toString()] = temps[instr.operands[0].toString()];
                } else if (instr.opcode == IROpCode::INPUT) {
                    // Print the prompt if provided
                    if (!instr.prompt.empty()) {
                        std::cout << instr.prompt;
                        std::cout.flush();
                    }
                    std::string input;
                    std::getline(std::cin, input);
                    temps[instr.result.toString()] = input;
                } else if (instr.opcode == IROpCode::KEY_PRESSED) {
                    // Read a single character without waiting for Enter
                    char key = readSingleKey();
                    std::string keyStr(1, key);
                    temps[instr.result.toString()] = keyStr;
                } else if (instr.opcode == IROpCode::SCREEN) {
                    // Screen initialization: create graphics window
                    if (instr.operands.size() >= 3) {
                        auto toInt = [](const Value& v) -> int {
                            if (std::holds_alternative<int>(v)) return std::get<int>(v);
                            if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                            if (std::holds_alternative<std::string>(v)) {
                                try { return std::stoi(std::get<std::string>(v)); } catch (...) { return 0; }
                            }
                            return 0;
                        };
                        
                        auto toString = [](const Value& v) -> std::string {
                            if (std::holds_alternative<std::string>(v)) return std::get<std::string>(v);
                            if (std::holds_alternative<int>(v)) return std::to_string(std::get<int>(v));
                            if (std::holds_alternative<double>(v)) return std::to_string(std::get<double>(v));
                            return "";
                        };
                        
                        int width = toInt(temps[instr.operands[0].toString()]);
                        int height = toInt(temps[instr.operands[1].toString()]);
                        std::string title = toString(temps[instr.operands[2].toString()]);
                        
                        try {
                            if (g_graphics) delete g_graphics;
                            g_graphics = new Graphics(width, height, title);
                            std::cout << "\033[2J\033[1;1H";  // Clear terminal
                            std::cout << "Graphics window created: " << width << "x" << height << " - " << title << std::endl;
                        } catch (const std::exception& e) {
                            std::cerr << "Failed to create graphics window: " << e.what() << std::endl;
                        }
                    }
                    temps[instr.result.toString()] = 1;  // Return success
                } else if (instr.opcode == IROpCode::DRAW_PIXEL) {
                    // drawPixel(x, y, r, g, b)
                    if (g_graphics && instr.operands.size() >= 5) {
                        auto toInt = [](const Value& v) -> int {
                            if (std::holds_alternative<int>(v)) return std::get<int>(v);
                            if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                            if (std::holds_alternative<std::string>(v)) {
                                try { return std::stoi(std::get<std::string>(v)); } catch (...) { return 0; }
                            }
                            return 0;
                        };
                        int x = toInt(temps[instr.operands[0].toString()]);
                        int y = toInt(temps[instr.operands[1].toString()]);
                        int r = toInt(temps[instr.operands[2].toString()]);
                        int g = toInt(temps[instr.operands[3].toString()]);
                        int b = toInt(temps[instr.operands[4].toString()]);
                        g_graphics->drawPixel(x, y, r, g, b);
                        temps[instr.result.toString()] = 1;
                    }
                } else if (instr.opcode == IROpCode::DRAW_RECT) {
                    // drawRect(x, y, w, h, r, g, b, filled)
                    if (g_graphics && instr.operands.size() >= 8) {
                        auto toInt = [](const Value& v) -> int {
                            if (std::holds_alternative<int>(v)) return std::get<int>(v);
                            if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                            if (std::holds_alternative<std::string>(v)) {
                                try { return std::stoi(std::get<std::string>(v)); } catch (...) { return 0; }
                            }
                            return 0;
                        };
                        int x = toInt(temps[instr.operands[0].toString()]);
                        int y = toInt(temps[instr.operands[1].toString()]);
                        int w = toInt(temps[instr.operands[2].toString()]);
                        int h = toInt(temps[instr.operands[3].toString()]);
                        int r = toInt(temps[instr.operands[4].toString()]);
                        int g = toInt(temps[instr.operands[5].toString()]);
                        int b = toInt(temps[instr.operands[6].toString()]);
                        int filled = toInt(temps[instr.operands[7].toString()]);
                        g_graphics->drawRect(x, y, w, h, r, g, b, filled);
                        temps[instr.result.toString()] = 1;
                    }
                } else if (instr.opcode == IROpCode::DRAW_LINE) {
                    // drawLine(x1, y1, x2, y2, r, g, b)
                    if (g_graphics && instr.operands.size() >= 7) {
                        auto toInt = [](const Value& v) -> int {
                            if (std::holds_alternative<int>(v)) return std::get<int>(v);
                            if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                            if (std::holds_alternative<std::string>(v)) {
                                try { return std::stoi(std::get<std::string>(v)); } catch (...) { return 0; }
                            }
                            return 0;
                        };
                        int x1 = toInt(temps[instr.operands[0].toString()]);
                        int y1 = toInt(temps[instr.operands[1].toString()]);
                        int x2 = toInt(temps[instr.operands[2].toString()]);
                        int y2 = toInt(temps[instr.operands[3].toString()]);
                        int r = toInt(temps[instr.operands[4].toString()]);
                        int g = toInt(temps[instr.operands[5].toString()]);
                        int b = toInt(temps[instr.operands[6].toString()]);
                        g_graphics->drawLine(x1, y1, x2, y2, r, g, b);
                        temps[instr.result.toString()] = 1;
                    }
                } else if (instr.opcode == IROpCode::DRAW_CIRCLE) {
                    // drawCircle(x, y, radius, r, g, b, filled)
                    if (g_graphics && instr.operands.size() >= 7) {
                        auto toInt = [](const Value& v) -> int {
                            if (std::holds_alternative<int>(v)) return std::get<int>(v);
                            if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                            if (std::holds_alternative<std::string>(v)) {
                                try { return std::stoi(std::get<std::string>(v)); } catch (...) { return 0; }
                            }
                            return 0;
                        };
                        int x = toInt(temps[instr.operands[0].toString()]);
                        int y = toInt(temps[instr.operands[1].toString()]);
                        int radius = toInt(temps[instr.operands[2].toString()]);
                        int r = toInt(temps[instr.operands[3].toString()]);
                        int g = toInt(temps[instr.operands[4].toString()]);
                        int b = toInt(temps[instr.operands[5].toString()]);
                        int filled = toInt(temps[instr.operands[6].toString()]);
                        g_graphics->drawCircle(x, y, radius, r, g, b, filled);
                        temps[instr.result.toString()] = 1;
                    }
                } else if (instr.opcode == IROpCode::CLEAR_SCREEN) {
                    // clearScreen(r, g, b) - Clear to color
                    if (g_graphics && instr.operands.size() >= 3) {
                        auto toInt = [](const Value& v) -> int {
                            if (std::holds_alternative<int>(v)) return std::get<int>(v);
                            if (std::holds_alternative<double>(v)) return static_cast<int>(std::get<double>(v));
                            if (std::holds_alternative<std::string>(v)) {
                                try { return std::stoi(std::get<std::string>(v)); } catch (...) { return 0; }
                            }
                            return 0;
                        };
                        int r = toInt(temps[instr.operands[0].toString()]);
                        int g = toInt(temps[instr.operands[1].toString()]);
                        int b = toInt(temps[instr.operands[2].toString()]);
                        g_graphics->clear(r, g, b);
                        temps[instr.result.toString()] = 1;
                    }
                } else if (instr.opcode == IROpCode::PRESENT) {
                    // present() - Update display
                    if (g_graphics) {
                        g_graphics->handleEvents();
                        g_graphics->present();
                        // Check if window was closed via X button or Alt+F4
                        if (g_graphics->shouldClose()) {
                            delete g_graphics;
                            g_graphics = nullptr;
                            ip = func.instructions.size();  // Exit loop
                            break;
                        }
                    }
                    temps[instr.result.toString()] = 1;
                } else if (instr.opcode == IROpCode::CALL && instr.label == "quit") {
                    // quit() - Clean exit
                    if (g_graphics) {
                        // Process pending events one final time
                        g_graphics->handleEvents();
                        delete g_graphics;
                        g_graphics = nullptr;
                    }
                    exit(0);
                } else if (instr.opcode == IROpCode::CALL && instr.label == "isKeyDown") {
                    // isKeyDown(keyCode) - returns 1 if key is down, 0 otherwise
                    int result = 0;
                    if (g_graphics && instr.operands.size() > 0) {
                        Value keyVal = temps[instr.operands[0].toString()];
                        std::string keyStr;
                        if (std::holds_alternative<std::string>(keyVal)) {
                            keyStr = std::get<std::string>(keyVal);
                        } else if (std::holds_alternative<int>(keyVal)) {
                            keyStr = std::to_string(std::get<int>(keyVal));
                        } else if (std::holds_alternative<double>(keyVal)) {
                            keyStr = std::to_string(static_cast<int>(std::get<double>(keyVal)));
                        }
                        
                        // Check specific keys
                        if (keyStr == "a") result = g_graphics->isKeyPressed(SDLK_a) ? 1 : 0;
                        else if (keyStr == "d") result = g_graphics->isKeyPressed(SDLK_d) ? 1 : 0;
                        else if (keyStr == "w") result = g_graphics->isKeyPressed(SDLK_w) ? 1 : 0;
                        else if (keyStr == "s") result = g_graphics->isKeyPressed(SDLK_s) ? 1 : 0;
                        else if (keyStr == "space") result = g_graphics->isKeyPressed(SDLK_SPACE) ? 1 : 0;
                        else if (keyStr == "left") result = g_graphics->isKeyPressed(SDLK_LEFT) ? 1 : 0;
                        else if (keyStr == "right") result = g_graphics->isKeyPressed(SDLK_RIGHT) ? 1 : 0;
                        else if (keyStr == "up") result = g_graphics->isKeyPressed(SDLK_UP) ? 1 : 0;
                        else if (keyStr == "down") result = g_graphics->isKeyPressed(SDLK_DOWN) ? 1 : 0;
                        else if (keyStr == "escape") result = g_graphics->isKeyPressed(SDLK_ESCAPE) ? 1 : 0;
                        
                        if (result == 1) {
                            std::cout << "Key detected: " << keyStr << std::endl;
                        }
                    }
                    temps[instr.result.toString()] = result;
                } else if (instr.opcode == IROpCode::CALL && instr.label == "updateInput") {
                    // updateInput() - manually update input state
                    if (g_graphics) {
                        g_graphics->handleEvents();
                    }
                    temps[instr.result.toString()] = 1;
                } else if (instr.opcode == IROpCode::LABEL) {
                    // No-op
                }
                
                ip++;
            }
        }
    }
}


int main(int argc, char* argv[]) {
    std::string source;
    if (argc > 1) {
        source = readFile(argv[1]);
    } else {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line == "END") break;
            source += line + "\n";
        }
    }
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto program = parser.parse();
        IRGenerator irgen(program);
        auto ir = irgen.generate();
        interpretIR(ir);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
