#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <memory>
#include <unordered_map>

// Graphics class: SDL2-based graphics rendering engine
// Provides 2D drawing primitives, image handling, and input management
class Graphics {
public:
    // Constructor: Initialize graphics system with window
    Graphics(int width, int height, const std::string& title);
    ~Graphics();
    
    // Window management
    bool isOpen() const { return open; }              // Check if window is open
    bool shouldClose() const { return !open; }       // Check if user closed window
    void handleEvents();                             // Process SDL events (input, close button)
    void clear(int r = 0, int g = 0, int b = 0);   // Clear screen to color
    void present();                                  // Update display with drawn content
    void close();                                    // Close the window
    void setTitle(const std::string& title);         // Change window title
    
    // Drawing primitives - all use RGB(A) color format (0-255)
    void drawPixel(int x, int y, int r, int g, int b, int a = 255);  // Draw single pixel
    void drawRect(int x, int y, int w, int h, int r, int g, int b, int filled = 0);      // Draw rectangle (outline or filled)
    void drawLine(int x1, int y1, int x2, int y2, int r, int g, int b);                 // Draw line
    void drawCircle(int x, int y, int radius, int r, int g, int b, int filled = 0);    // Draw circle
    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int filled = 0);  // Draw triangle
    void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b);  // Draw filled triangle
    
    // Filled shape helpers
    void fillRect(int x, int y, int w, int h, int r, int g, int b);    // Draw filled rectangle
    void fillCircle(int x, int y, int radius, int r, int g, int b);   // Draw filled circle
    
    // Image/Texture support for sprite rendering
    bool loadImage(const std::string& filename, const std::string& name);  // Load image file
    void blitImage(const std::string& name, int x, int y);                 // Draw image at position
    void blitImageScaled(const std::string& name, int x, int y, int w, int h);  // Draw scaled image
    void freeImage(const std::string& name);                              // Free single image
    void freeAllImages();                                                 // Free all loaded images
    bool imageExists(const std::string& name) const;                      // Check if image is loaded
    
    // Text rendering (basic support)
    void drawText(const std::string& text, int x, int y, int r, int g, int b);
    
    // Input handling
    bool isKeyPressed(SDL_Keycode key);          // Check if key is currently pressed
    bool getMousePos(int& x, int& y);            // Get current mouse position
    bool isMouseButtonDown(int button = 1);      // Check if mouse button is pressed (1=left, 2=middle, 3=right)
    
    // Property accessors
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    SDL_Renderer* getRenderer() const { return renderer; }
    SDL_Window* getWindow() const { return window; }
    
private:
    SDL_Window* window;                                          // SDL window pointer
    SDL_Renderer* renderer;                                      // SDL 2D renderer
    std::unordered_map<std::string, SDL_Texture*> textures;     // Loaded image cache
    int width, height;                                           // Window dimensions
    bool open;                                                   // Whether window is open
    const Uint8* keyState;                                       // Current keyboard state
    
    // Helper for midpoint circle algorithm
    void midpointCircle(int x, int y, int radius, int r, int g, int b);
};

#endif // GRAPHICS_H
