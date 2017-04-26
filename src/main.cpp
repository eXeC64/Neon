#include <iostream>
#include <SDL2/SDL.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

#include "OpenGL.hpp"
#include "Renderer.hpp"
#include "Loader.hpp"
#include "Model.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"
#include "Material.hpp"

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            GLuint id,
                            GLenum severity,
                            GLsizei length,
                            const GLchar *message,
                            const void *userParam)
{
  (void)length;
  (void)userParam;
  // ignore non-significant error/warning codes
  if(id == 131204 || id == 131185)
    return;
  /* if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; */

  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " <<  message << std::endl;

  switch (source)
  {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
  } std::cout << std::endl;

  switch (type)
  {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
  } std::cout << std::endl;

  switch (severity)
  {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
  } std::cout << std::endl;
  std::cout << std::endl;
}

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

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(&glDebugOutput, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

  ne::Renderer* pRenderer = new ne::Renderer();

  if(!pRenderer->Init(width,height))
  {
    std::cerr << "Failed to init renderer." << std::endl;
    return 1;
  }

  pRenderer->SetGlobalIllumination(glm::vec3(0.2));

  ne::Loader loader;

  /* ne::Model *pModel = loader.LoadModel("meshes/nanosuit.obj"); */
  /* ne::Model *pModel = loader.LoadModel("meshes/sibenik.obj"); */
  ne::Model *pModel = loader.LoadModel("meshes/sponza.obj");
  /* ne::Model *pModel = loader.LoadModel("meshes/shipment.dae"); */
  /* ne::Model *pModel = loader.LoadModel("meshes/teapot.dae"); */

  ne::Mesh *pCube = loader.GenerateCube();
  ne::Texture *pDif = loader.LoadTexture("images/bricks_dif.png", ne::TextureFormat::Color);
  ne::Texture *pNorm = loader.LoadTexture("images/bricks_norm.png", ne::TextureFormat::Normal);
  ne::Material matStone(pDif, pNorm);

  glm::vec3 cameraPos(10,7,0);
  float cameraYaw = -1.5;
  float cameraTilt = 0;

  double lastTime = SDL_GetTicks() / 1000.0;
  double lightTime = 0.0;

  bool quit = false;
  while(!quit)
  {
    double curTime = SDL_GetTicks() / 1000.0;
    double dt = curTime - lastTime;
    lastTime = curTime;

    glm::vec3 cameraNorm = -glm::vec3(glm::vec4(0,0,1,0) *
      glm::rotate(glm::mat4(1.0), cameraTilt, glm::vec3(1,0,0)) *
      glm::rotate(glm::mat4(1.0), cameraYaw, glm::vec3(0,1,0)));

    pRenderer->SetViewPosition(cameraPos, cameraYaw, cameraTilt);

    pRenderer->AddTime(dt);

    pRenderer->BeginFrame();
    for(size_t i = 0; i < pModel->m_meshes.size(); ++i)
    {
      pRenderer->AddMesh(pModel->m_meshes[i], pModel->m_materials[i], glm::mat4(1.0));
    }
    pRenderer->AddMesh(pCube, &matStone, glm::translate(glm::mat4(1.0), glm::vec3(0, 3, 0)));


    /* pRenderer->AddLight( */
    /*     cameraPos, */
    /*     glm::vec3(1.0, 1.0, 1.0) */
    /* ); */

    /* glm::vec3 lightPos(9 * glm::sin(2*lightTime), -10.0, 5.5 * glm::cos(2*lightTime)); */
    glm::vec3 lightPos(9 * glm::sin(2*lightTime), 5.5, 5.5 * glm::cos(2*lightTime));
    pRenderer->AddLight(lightPos, glm::vec3(1.0, 1.0, 1.0));

    {
      glm::mat4 sScale = glm::scale(glm::mat4(1.0), glm::vec3(5));
      glm::mat4 cScale = glm::scale(glm::mat4(1.0), glm::vec3(0.5));
      glm::mat4 tran = glm::translate(glm::mat4(1.0), lightPos);
      pRenderer->AddDebugSphere(tran * sScale, glm::vec3(1.0, 1.0, 1.0));
      pRenderer->AddDebugCube(tran * cScale, glm::vec3(1.0, 0.0, 0.0));
    }

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
          cameraYaw += 0.15 * dt * e.motion.xrel;
          cameraTilt += 0.15 * dt * e.motion.yrel;
          break;
        case SDL_KEYDOWN:
          if(e.key.keysym.sym == SDLK_p) {
            std::cout << "cameraPos: " << cameraPos.x << "," << cameraPos.y << "," << cameraPos.z << std::endl;
          }
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
      cameraPos += glm::vec3(moveSpeed * dt) * glm::vec3(0,1,0);
    if(keyboard[SDL_SCANCODE_LCTRL])
      cameraPos -= glm::vec3(moveSpeed * dt) * glm::vec3(0,1,0);
    if(keyboard[SDL_SCANCODE_T])
    {
      if(keyboard[SDL_SCANCODE_LSHIFT])
        lightTime -= dt;
      else
        lightTime += dt;
    }


  }

  delete pRenderer;

  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(pWindow);
  SDL_Quit();
  return 0;
}
