#include "graphics.h"
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <algorithm>

Graphics::Graphics(int w, int h, const std::string& title)
    : width(w), height(h), open(false), window(nullptr), renderer(nullptr), keyState(nullptr) {
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
    }
    
    // Initialize SDL_image for image loading
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_image initialization failed: ") + IMG_GetError());
    }
    
    window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        SDL_Quit();
        throw std::runtime_error(std::string("Window creation failed: ") + SDL_GetError());
    }
    
    renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error(std::string("Renderer creation failed: ") + SDL_GetError());
    }
    
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    open = true;
}

Graphics::~Graphics() {
    freeAllImages();
    close();
}

void Graphics::close() {
    freeAllImages();
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    IMG_Quit();
    SDL_Quit();
    open = false;
}

void Graphics::setTitle(const std::string& title) {
    if (window) {
        SDL_SetWindowTitle(window, title.c_str());
    }
}

void Graphics::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            open = false;
        } else if (event.type == SDL_KEYDOWN) {
            keyState = SDL_GetKeyboardState(nullptr);
        } else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
            // Mouse events handled in isMouseButtonDown
        }
    }
}

void Graphics::clear(int r, int g, int b) {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderClear(renderer);
}

void Graphics::present() {
    if (!renderer) return;
    SDL_RenderPresent(renderer);
}

void Graphics::drawPixel(int x, int y, int r, int g, int b, int a) {
    if (!renderer || x < 0 || y < 0 || x >= width || y >= height) return;
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawPoint(renderer, x, y);
}

void Graphics::drawRect(int x, int y, int w, int h, int r, int g, int b, int filled) {
    if (!renderer) return;
    SDL_Rect rect = {x, y, w, h};
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    if (filled) {
        SDL_RenderFillRect(renderer, &rect);
    } else {
        SDL_RenderDrawRect(renderer, &rect);
    }
}

void Graphics::fillRect(int x, int y, int w, int h, int r, int g, int b) {
    drawRect(x, y, w, h, r, g, b, 1);
}

void Graphics::drawLine(int x1, int y1, int x2, int y2, int r, int g, int b) {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

void Graphics::midpointCircle(int x, int y, int radius, int r, int g, int b) {
    int x0 = 0;
    int y0 = radius;
    int d = 3 - 2 * radius;
    
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    while (x0 <= y0) {
        SDL_RenderDrawPoint(renderer, x + x0, y + y0);
        SDL_RenderDrawPoint(renderer, x - x0, y + y0);
        SDL_RenderDrawPoint(renderer, x + x0, y - y0);
        SDL_RenderDrawPoint(renderer, x - x0, y - y0);
        SDL_RenderDrawPoint(renderer, x + y0, y + x0);
        SDL_RenderDrawPoint(renderer, x - y0, y + x0);
        SDL_RenderDrawPoint(renderer, x + y0, y - x0);
        SDL_RenderDrawPoint(renderer, x - y0, y - x0);
        
        if (d < 0) {
            d = d + 4 * x0 + 6;
        } else {
            d = d + 4 * (x0 - y0) + 10;
            y0--;
        }
        x0++;
    }
}

void Graphics::drawCircle(int x, int y, int radius, int r, int g, int b, int filled) {
    if (!renderer) return;
    
    if (filled) {
        for (int ry = -radius; ry <= radius; ry++) {
            int rx = (int)std::sqrt(radius * radius - ry * ry);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderDrawLine(renderer, x - rx, y + ry, x + rx, y + ry);
        }
    } else {
        midpointCircle(x, y, radius, r, g, b);
    }
}

void Graphics::fillCircle(int x, int y, int radius, int r, int g, int b) {
    drawCircle(x, y, radius, r, g, b, 1);
}

void Graphics::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int filled) {
    if (!renderer) return;
    
    if (filled) {
        fillTriangle(x1, y1, x2, y2, x3, y3, r, g, b);
    } else {
        drawLine(x1, y1, x2, y2, r, g, b);
        drawLine(x2, y2, x3, y3, r, g, b);
        drawLine(x3, y3, x1, y1, r, g, b);
    }
}

void Graphics::fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b) {
    if (!renderer) return;
    
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    
    int minY = std::min(std::min(y1, y2), y3);
    int maxY = std::max(std::max(y1, y2), y3);
    
    for (int y = minY; y <= maxY; y++) {
        int minX = 1000000, maxX = -1000000;
        
        // Check edge 1-2
        if (y1 != y2) {
            double x = x1 + (double)(y - y1) * (x2 - x1) / (y2 - y1);
            minX = std::min(minX, (int)x);
            maxX = std::max(maxX, (int)x);
        }
        
        // Check edge 2-3
        if (y2 != y3) {
            double x = x2 + (double)(y - y2) * (x3 - x2) / (y3 - y2);
            minX = std::min(minX, (int)x);
            maxX = std::max(maxX, (int)x);
        }
        
        // Check edge 3-1
        if (y3 != y1) {
            double x = x3 + (double)(y - y3) * (x1 - x3) / (y1 - y3);
            minX = std::min(minX, (int)x);
            maxX = std::max(maxX, (int)x);
        }
        
        if (minX <= maxX) {
            SDL_RenderDrawLine(renderer, minX, y, maxX, y);
        }
    }
}

// Image/Texture support
bool Graphics::loadImage(const std::string& filename, const std::string& name) {
    if (!renderer) return false;
    
    if (textures.find(name) != textures.end()) {
        SDL_DestroyTexture(textures[name]);
    }
    
    SDL_Surface* surface = IMG_Load(filename.c_str());
    if (!surface) {
        std::cerr << "Error loading image " << filename << ": " << IMG_GetError() << std::endl;
        return false;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        std::cerr << "Error creating texture from " << filename << ": " << SDL_GetError() << std::endl;
        return false;
    }
    
    textures[name] = texture;
    return true;
}

void Graphics::blitImage(const std::string& name, int x, int y) {
    if (!renderer || textures.find(name) == textures.end()) return;
    
    SDL_Texture* texture = textures[name];
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    
    SDL_Rect dstRect = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
}

void Graphics::blitImageScaled(const std::string& name, int x, int y, int w, int h) {
    if (!renderer || textures.find(name) == textures.end()) return;
    
    SDL_Texture* texture = textures[name];
    SDL_Rect dstRect = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
}

void Graphics::freeImage(const std::string& name) {
    if (textures.find(name) != textures.end()) {
        SDL_DestroyTexture(textures[name]);
        textures.erase(name);
    }
}

void Graphics::freeAllImages() {
    for (auto& pair : textures) {
        SDL_DestroyTexture(pair.second);
    }
    textures.clear();
}

bool Graphics::imageExists(const std::string& name) const {
    return textures.find(name) != textures.end();
}

void Graphics::drawText(const std::string& text, int x, int y, int r, int g, int b) {
    // Basic text rendering - in a full implementation, would use SDL_ttf
    // For now, just a placeholder
    (void)text; (void)x; (void)y; (void)r; (void)g; (void)b;
}

bool Graphics::isKeyPressed(SDL_Keycode key) {
    keyState = SDL_GetKeyboardState(nullptr);
    SDL_Scancode scancode = SDL_GetScancodeFromKey(key);
    return keyState != nullptr && keyState[scancode] != 0;
}

bool Graphics::getMousePos(int& x, int& y) {
    return SDL_GetMouseState(&x, &y) != 0;
}

bool Graphics::isMouseButtonDown(int button) {
    int mouseState = SDL_GetMouseState(nullptr, nullptr);
    switch (button) {
        case 1: return (mouseState & SDL_BUTTON_LMASK) != 0;  // Left
        case 2: return (mouseState & SDL_BUTTON_MMASK) != 0;  // Middle
        case 3: return (mouseState & SDL_BUTTON_RMASK) != 0;  // Right
        default: return false;
    }
}
