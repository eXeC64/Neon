#include <iostream>
#include <SDL2/SDL.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

#include "OpenGL.hpp"
#include "Renderer.hpp"
#include "Loader.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Material.hpp"

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  if(SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    std::cerr << "SDL Failed to Init: " << SDL_GetError() << std::endl;
    return 1;
  }
  const int width = 1920;
  const int height = 1080;

  SDL_Window *pWindow = SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);

  if(!pWindow)
  {
    std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
    return 1;
  }
  SDL_SetRelativeMouseMode(SDL_TRUE);

  SDL_GLContext GLcontext = SDL_GL_CreateContext(pWindow);

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
  /* std::cout << "Extensions: " << std::endl; */
  /* GLint numExts; */
  /* glGetIntegerv(GL_NUM_EXTENSIONS, &numExts); */
  /* for(int i = 0; i < numExts; ++i) */
  /*   std::cout << " * " << glGetStringi(GL_EXTENSIONS, i) << "\n"; */
  /* std::cout << std::flush; */

  he::Renderer* pRenderer = new he::Renderer();

  if(!pRenderer->Init(width,height))
  {
    std::cerr << "Failed to init renderer." << std::endl;
    return 1;
  }


  he::Texture *pDiffuse = he::Loader::LoadPNG("diffuse.png");
  he::Texture *pNormal = he::Loader::LoadPNG("normal.png");
  he::Material mat(pDiffuse, pNormal);
  he::Mesh *pMesh = he::Loader::LoadOBJ("sponza.obj");

  glm::vec3 cameraPos(0,-2,0);
  float cameraYaw = 0;
  float cameraTilt = 0;

  double lastTime = SDL_GetTicks() / 1000.0;

  bool quit = false;
  while(!quit)
  {
    double curTime = SDL_GetTicks() / 1000.0;
    double dt = curTime - lastTime;
    lastTime = curTime;

    glm::vec3 cameraNorm = glm::vec3(glm::vec4(0,0,1,0) *
      glm::rotate(glm::mat4(1.0), cameraTilt, glm::vec3(1,0,0)) *
      glm::rotate(glm::mat4(1.0), cameraYaw, glm::vec3(0,1,0)));

    pRenderer->SetViewPosition(cameraPos, cameraYaw, cameraTilt);

    pRenderer->AddTime(dt);
    pRenderer->BeginFrame();
    pRenderer->AddMesh(pMesh, &mat, glm::mat4(1.0));
    pRenderer->EndFrame();

    SDL_GL_SwapWindow(pWindow);
    SDL_Delay(10);

    //Handle events
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
      switch(e.type)
      {
        case SDL_QUIT:
          quit = true;
          break;
        case SDL_MOUSEMOTION:
          cameraYaw += 0.05 * dt * e.motion.xrel;
          cameraTilt += 0.05 * dt * e.motion.yrel;
          break;
      }
    }

    const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
    float moveSpeed = keyboard[SDL_SCANCODE_LSHIFT] ? 1 : 5;
    if(keyboard[SDL_SCANCODE_W])
      cameraPos += glm::vec3(moveSpeed * dt) * cameraNorm;
    if(keyboard[SDL_SCANCODE_S])
      cameraPos -= glm::vec3(moveSpeed * dt) * cameraNorm;
    if(keyboard[SDL_SCANCODE_A])
      cameraPos -= glm::vec3(moveSpeed * dt) * glm::cross(cameraNorm,glm::vec3(0,1,0));
    if(keyboard[SDL_SCANCODE_D])
      cameraPos += glm::vec3(moveSpeed * dt) * glm::cross(cameraNorm,glm::vec3(0,1,0));
    if(keyboard[SDL_SCANCODE_SPACE])
      cameraPos -= glm::vec3(moveSpeed * dt) * glm::vec3(0,1,0);
    if(keyboard[SDL_SCANCODE_LCTRL])
      cameraPos += glm::vec3(moveSpeed * dt) * glm::vec3(0,1,0);


  }

  delete pDiffuse;
  delete pNormal;
  delete pMesh;
  delete pRenderer;

  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(pWindow);
  SDL_Quit();
  return 0;
}
