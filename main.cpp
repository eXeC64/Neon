#include <iostream>
#include <SDL2/SDL.h>

int main(int argc, char **argv)
{
  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    std::cerr << "SDL Failed to Init: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Window *pWindow = SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);

  if(!pWindow)
  {
    std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_GLContext GLcontext = SDL_GL_CreateContext(pWindow);
  glClearColor(0,0,0,1);

  bool quit = false;
  while(!quit)
  {
    double curtime = SDL_GetTicks() / 1000.0;

    glClear(GL_COLOR_BUFFER_BIT);
    //Draw here
    SDL_GL_SwapWindow(pWindow);
    SDL_Delay(1);

    //Handle events
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
      switch(e.type)
      {
        case SDL_QUIT:
          quit = true;
          break;
      }
    }
  }

  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(pWindow);
  SDL_Quit();
  return 0;
}
