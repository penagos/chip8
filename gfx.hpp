#include <bits/stdint-uintn.h>

#include <SDL2/SDL.h> 

namespace chip8 {
// Handle all SDL related things
class Gfx {
    public:
        Gfx(char const* title, int windowWidth, int windowHeight,
            int textureWidth, int textureHeight)
        : title(title)
        , windowWidth(windowWidth)
        , windowHeight(windowHeight)
        , textureWidth(textureWidth) {
            // Initialize SDL gfx
            SDL_Init(SDL_INIT_VIDEO);

            window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight,
                                        SDL_WINDOW_SHOWN);
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_STREAMING, textureWidth,
                                        textureHeight);
        }

        void update(void const* buffer, int pitch);
        bool input(uint8_t* keys);
        ~Gfx();
    
    private:
        char const* title;
        int windowWidth;
        int windowHeight;
        int textureWidth;
        int textureHeight;

        SDL_Window* window{};
        SDL_Renderer* renderer{};
        SDL_Texture* texture{};
};
}