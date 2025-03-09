#include <SDL2/SDL.h>
#include <random>
#include <iostream>
#include <string>
#include <cmath>

using namespace std;

// Pattern type enum
enum PatternType {
    ORIGINAL,
    POLAR,
    TRIGONOMETRIC,
    FRACTAL,
    WAVE_INTERFERENCE,
    SYMMETRY
};

int main(int argc, char *argv[]){
  if(argc < 5){
    std::cout << "USAGE: randomart <width> <height> <pixelsize> <mode> [seizure_mode]" << std::endl;
    std::cout << "Mode is either 0 or 1" << std::endl;
    std::cout << "Optional seizure_mode is 0 or 1 (default: 0)" << std::endl;
    exit(1);
  }
  int Width = atoi(argv[1]);
  int Height = atoi(argv[2]);
  int tilesize = atoi(argv[3]);
  int mode = atoi(argv[4]);
  int seizure_mode = (argc > 5) ? atoi(argv[5]) : 0;
  cout << Width << " " << Height << " " << tilesize << endl;

  std::random_device rdev;
  unsigned long randseed = rdev();
  SDL_Window* window = SDL_CreateWindow("Artmaker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, 0);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawColor(renderer, 0,0,0,0);
  SDL_RenderClear(renderer);
  SDL_RenderPresent(renderer);
  auto gen_and_ren = [renderer, Width, Height, tilesize, mode](unsigned long randseed, PatternType pattern_type){
    // Create a single random generator for efficiency
    std::mt19937 random{static_cast<std::mt19937::result_type>(randseed)};
    std::uniform_int_distribution<int> dist(0, 255);
    
    // Center coordinates for polar patterns
    int centerX = Width / 2;
    int centerY = Height / 2;
    
    SDL_RenderClear(renderer);
    
    for(int i = 0; i < Width; i += tilesize){
        for(int j = 0; j < Height; j += tilesize){
            // Different seed calculation based on pattern type
            unsigned long pattern_seed;
            
            switch(pattern_type) {
                case ORIGINAL:
                    pattern_seed = randseed | (i*j & i*j);
                    break;
                case POLAR: {
                    // Distance and angle from center
                    float dx = i - centerX;
                    float dy = j - centerY;
                    float distance = sqrt(dx*dx + dy*dy);
                    float angle = atan2(dy, dx);
                    pattern_seed = randseed | (static_cast<int>(distance*10) & static_cast<int>(angle*1000));
                    break;
                }
                case TRIGONOMETRIC:
                    pattern_seed = randseed | (static_cast<int>(sin(i*0.05)*100) * static_cast<int>(cos(j*0.05)*100));
                    break;
                case FRACTAL:
                    // Multi-scale pattern
                    pattern_seed = randseed | ((i*j & i*j) ^ ((i/8)*(j/8) & (i/8)*(j/8)));
                    break;
                case WAVE_INTERFERENCE: {
                    // Multiple wave sources
                    float wave1 = sin(i*0.05 + j*0.05) * 100;
                    float wave2 = sin(i*0.08 - j*0.03) * 100;
                    float wave3 = sin(sqrt(pow(i-Width/2, 2) + pow(j-Height/2, 2)) * 0.1) * 100;
                    
                    // Combine waves
                    pattern_seed = randseed | (static_cast<int>(wave1 + wave2 + wave3));
                    break;
                }
                case SYMMETRY: {
                    // Create 8-fold symmetry
                    int x = i % (Width/2);
                    int y = j % (Height/2);
                    if (i >= Width/2) x = Width/2 - x;
                    if (j >= Height/2) y = Height/2 - y;
                    
                    pattern_seed = randseed | (x*y & x*y);
                    break;
                }
            }
            
            random.seed(pattern_seed);
            
            // Generate color
            unsigned char r = dist(random);
            unsigned char g = dist(random);
            unsigned char b = dist(random);
            unsigned char a = 255; // Full opacity
            
            SDL_Rect dest_rect = {i, j, tilesize, tilesize};
            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            
            if(mode==0){
                SDL_RenderDrawRect(renderer, &dest_rect);
            }
            else {
                SDL_RenderFillRect(renderer, &dest_rect);
            }
        }
    }
    
    SDL_RenderPresent(renderer);
  };

  SDL_Event event;
  for(;;){
    SDL_PollEvent(&event);
    if(seizure_mode){
      gen_and_ren(rdev(), ORIGINAL);
    }
    switch (event.type) {
      case SDL_QUIT:
        exit(0);
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
            case SDLK_SPACE:
              randseed = rdev();
              gen_and_ren(randseed, ORIGINAL);
              break;
            case SDLK_ESCAPE:
              exit(0);
              break;
            default:
              gen_and_ren(randseed | (event.key.keysym.sym * 10), SYMMETRY);
          }
      }
    }
}
