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

  SDL_Window *pWindow = SDL_CreateWindow("game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);

  if(!pWindow)
  {
    std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_GLContext GLcontext = SDL_GL_CreateContext(pWindow);

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
  /* std::cout << "Extensions: " << std::endl; */
  /* GLint numExts; */
  /* glGetIntegerv(GL_NUM_EXTENSIONS, &numExts); */
  /* for(int i = 0; i < numExts; ++i) */
  /*   std::cout << " * " << glGetStringi(GL_EXTENSIONS, i) << "\n"; */
  /* std::cout << std::flush; */

  he::Renderer* pRenderer = new he::Renderer();

  if(!pRenderer->Init(1280,720))
  {
    std::cerr << "Failed to init renderer." << std::endl;
    return 1;
  }

  glm::mat4 proj = glm::perspective(45.0, 16.0/9.0, 0.1, 100.0);
  glm::mat4 tran = glm::rotate(glm::mat4(1.0), 0 * glm::pi<float>(), glm::vec3(0,1,0))
    * glm::translate(glm::mat4(1.0), glm::vec3(0,0,-5));
  pRenderer->SetProjectionMatrix(proj * tran);

  he::Texture *pDiffuse = he::Loader::LoadPNG("diffuse.png");
  he::Texture *pNormal = he::Loader::LoadPNG("normal.png");
  he::Material mat(pDiffuse, pNormal);
  he::Mesh *pMesh = he::Loader::LoadOBJ("monkey.obj");

  bool quit = false;
  while(!quit)
  {
    double curtime = SDL_GetTicks() / 1000.0;

    glm::mat4 matMeshPos = glm::translate(glm::mat4(1.0), glm::vec3(0,0,1 + 1*sin(3*curtime)))
        * glm::rotate(glm::mat4(1.0), (float)curtime * 0, glm::vec3(0,1,0));

    pRenderer->BeginFrame();
    pRenderer->AddMesh(pMesh, &mat, matMeshPos);
    pRenderer->EndFrame();

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

  delete pDiffuse;
  delete pNormal;
  delete pMesh;
  delete pRenderer;

  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(pWindow);
  SDL_Quit();
  return 0;
}