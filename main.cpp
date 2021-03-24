#include <SDL2/SDL.h>
#include <random>
#include <iostream>
#include <string>

using namespace std;






int main(int argc, char *argv[]){
  int Width = atoi(argv[1]);
  int Height = atoi(argv[2]);
  int tilesize = atoi(argv[3]);
  cout << Width << " " << Height << " " << tilesize << endl;
  std::random_device rdev;
  unsigned long randseed = rdev();
  SDL_Window* window = SDL_CreateWindow("Artmaker", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, 0);
  SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawColor(renderer, 0,0,0,0);
  SDL_RenderClear(renderer);
  auto gen_and_ren = [renderer, Width, Height, tilesize](unsigned long randseed){
    for(int i = 0; i < Width; i += tilesize){
      for(int j = 0; j < Height; j += tilesize){
        int xx;
        if(i > Width/2)
          xx = i * -1;
        else
         xx = i;

        std::mt19937 random{randseed  | xx | j};
        auto set_colors = [&random]() -> SDL_Color{
          std::uniform_int_distribution<int> dist(0, 255);
          unsigned char r = dist(random);
          unsigned char g = dist(random);
          unsigned char b = dist(random);
          unsigned char a = dist(random);
          SDL_Color rgb = {r, g, b, 0};
          return rgb;
        };
        SDL_Color color = set_colors();
        SDL_Rect dest_rect = {i, j, tilesize, tilesize};
        SDL_SetRenderDrawColor(renderer, color.r, color.b, color.g, color.a);
        SDL_RenderFillRect(renderer, &dest_rect);
        }
      }
      SDL_RenderPresent(renderer);
  };
  gen_and_ren(randseed);


  SDL_Event event;
  for(;;){
    SDL_PollEvent(&event);
    switch (event.type) {
      case SDL_QUIT:
        exit(0);
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
            case SDLK_SPACE:
              gen_and_ren(rdev());
              std::cout << "aaaaaa" << '\n';
              break;
            case SDLK_ESCAPE:
              exit(0);
              break;
            default:
              break;

          }
      }
    }

}
