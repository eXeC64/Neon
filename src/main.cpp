#include <iostream>
#include <SDL2/SDL.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <vector>

#include "OpenGL.hpp"
#include "Renderer.hpp"
#include "Loader.hpp"
#include "StaticModel.hpp"
#include "StaticMesh.hpp"
#include "AnimatedModel.hpp"
#include "AnimatedMesh.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "ImguiWrapper.hpp"

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

struct Light
{
  bool enabled;
  glm::vec3 pos;
  glm::vec3 color;
  float bright;
};

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

  ne::ImguiWrapper gui;

  ne::Loader loader;

  ne::StaticModel *sponza = loader.LoadStaticModel("meshes/sponza.obj");
  ne::StaticModel *nanosuit = loader.LoadStaticModel("meshes/nanosuit.obj");
  ne::AnimatedMesh *cowboy = loader.LoadAnimatedMesh("cowboy.mesh");
  ne::Skeleton *cowboySkel = loader.LoadSkeleton("cowboy.skel");

  glm::vec3 cameraPos(10,7,0);
  float cameraYaw = -1.5;
  float cameraTilt = 0;

  double lastTime = SDL_GetTicks() / 1000.0;
  double lightTime = 0.0;

  std::vector<Light> lights;
  lights.push_back(Light{true, glm::vec3(-5,3,0), glm::vec3(1), 5.0f});
  lights.push_back(Light{true, glm::vec3( 5,3,0), glm::vec3(1), 5.0f});

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
    gui.NewFrame(pWindow);

    for(size_t i = 0; i < sponza->m_meshes.size(); ++i)
      pRenderer->AddStaticMesh(sponza->m_meshes[i], sponza->m_materials[i], glm::mat4(1.0));

    {
      float scale = 0.13;
      float rot = 90.0;
      glm::mat4 nanoScale = glm::scale(glm::mat4(1), glm::vec3(scale));
      glm::mat4 nanoRot = glm::rotate(glm::mat4(1), glm::radians(rot), glm::vec3(0,1,0));
      /* for(size_t i = 0; i < nanosuit->m_meshes.size(); ++i) */
        /* pRenderer->AddStaticMesh(nanosuit->m_meshes[i], nanosuit->m_materials[i], nanoScale * nanoRot); */
    }

    {
      float scale = 0.30;
      float rot = -90.0;
      glm::mat4 matScale = glm::scale(glm::mat4(1), glm::vec3(scale));
      glm::mat4 matRot = glm::rotate(glm::mat4(1), glm::radians(rot), glm::vec3(1,0,0));
      static std::vector<glm::mat4> boneTransforms(32, glm::mat4(1.0));
      /* boneTransforms[0] = glm::mat4(1.0); */
      /* static float headOffset = 0.0; */
      ImGui::SliderFloat("Head height", &cowboySkel->bones[9].localPos.z, 0.0, 5.0);
      cowboySkel->calculateInvTransforms(boneTransforms);
      /* boneTransforms[9] = glm::translate(glm::mat4(1.0), glm::vec3(0.0f,0.0f,headOffset)); */

      pRenderer->AddAnimatedMesh(cowboy, nullptr, matScale * matRot, &boneTransforms);
    }


    static float globalIllum = 0.025;
    static float gamma = 2.2;
    static float exposure = 1.0;

    static bool cameraLight = false;
    static glm::vec3 cameraLightCol(1.0);

    static bool floatingLight = false;
    static glm::vec3 floatingLightCol(1.0);
    static bool floatingLightSphere = false;

    static bool debugMenu = false;
    static bool positionOverlay = false;
    static bool fpsOverlay = false;
    static bool lightsMenu = false;
    static bool sun = false;
    static float sunStrength = 0.5;
    static glm::vec3 sunDir(1,2,1);
    static glm::vec3 sunCol(1,1,0.5);


    glm::vec3 lightPos(9 * glm::sin(2*lightTime), 5.5, 5.5 * glm::cos(2*lightTime));

    if(ImGui::BeginMainMenuBar())
    {
      if(ImGui::BeginMenu("Tools"))
      {
        ImGui::MenuItem("Debug", NULL, debugMenu);
        if(ImGui::IsItemClicked())
          debugMenu = !debugMenu;

        ImGui::MenuItem("Show FPS", NULL, fpsOverlay);
        if(ImGui::IsItemClicked())
          fpsOverlay = !fpsOverlay;

        ImGui::MenuItem("Position Overlay", NULL, positionOverlay);
        if(ImGui::IsItemClicked())
          positionOverlay = !positionOverlay;

        ImGui::MenuItem("Lights", NULL, lightsMenu);
        if(ImGui::IsItemClicked())
          lightsMenu = !lightsMenu;

        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    if(debugMenu)
    {
      ImGui::Begin("Lighting", &debugMenu, ImGuiWindowFlags_AlwaysAutoResize);

      /* ImGui::PushID("camera"); */
      ImGui::Checkbox("Camera Light", &cameraLight);
      ImGui::ColorEdit3("Color##camera", &cameraLightCol.x);
      /* ImGui::PopID(); */
      ImGui::Separator();
      /* ImGui::PushID("floating"); */
      ImGui::Checkbox("Floating Light", &floatingLight);
      ImGui::ColorEdit3("Color##floating", &floatingLightCol.x);
      ImGui::Checkbox("Draw Outline", &floatingLightSphere);
      /* ImGui::PopID(); */
      ImGui::End();
    }

    if(positionOverlay)
    {
      ImGui::SetNextWindowPos(ImVec2(10,40));
      ImGui::Begin("Position Overlay",
                       NULL,
                       ImVec2(0,0),
                       0.5f,
                       ImGuiWindowFlags_NoTitleBar
                         |ImGuiWindowFlags_NoResize
                         |ImGuiWindowFlags_NoMove
                         |ImGuiWindowFlags_NoSavedSettings);
      ImGui::Text("Camera Position: %8.3f %8.3f %8.3f", cameraPos.x, cameraPos.y, cameraPos.z);
      ImGui::Text(" Light Position: %8.3f %8.3f %8.3f", lightPos.x, lightPos.y, lightPos.z);
      ImGui::End();
    }

    if(fpsOverlay)
    {
      ImGui::SetNextWindowPos(ImVec2(10,40));
      ImGui::Begin("FPS",
                       NULL,
                       ImVec2(0,0),
                       0.5f,
                       ImGuiWindowFlags_NoTitleBar
                         |ImGuiWindowFlags_NoResize
                         |ImGuiWindowFlags_NoMove
                         |ImGuiWindowFlags_NoSavedSettings);
      ImGui::Text("FPS: %.0f", 1.0 / dt);
      ImGui::Text("Frame Time: %1.3fms", dt);
      ImGui::End();
    }

    if(lightsMenu)
    {
      ImGui::Begin("Lights", &lightsMenu, ImGuiWindowFlags_AlwaysAutoResize);

      ImGui::SliderFloat("Global Illumination", &globalIllum, 0.0, 0.02);
      ImGui::SliderFloat("Gamma", &gamma, 1.0, 3.0);
      ImGui::SliderFloat("Exposure", &exposure, 0.01, 10.0);
      ImGui::Separator();
      ImGui::Checkbox("Sun", &sun);
      ImGui::SliderFloat("Sun Strength", &sunStrength, 0.0, 1.0);
      ImGui::DragFloat3("Sun Direction", &sunDir.x, 0.01);
      ImGui::ColorEdit3("Sun Color", &sunCol.x);
      ImGui::Separator();
      ImGui::Button("Add Light");
      if(ImGui::IsItemClicked())
      {
        lights.push_back(Light{true, glm::vec3{0, 2, 0}, glm::vec3{1}, 1.0});
      }

      for(size_t i = 0; i < lights.size(); ++i)
      {
        Light& light = lights[i];
        ImGui::Separator();
        ImGui::PushID(&light);
        ImGui::Checkbox("Enabled", &light.enabled);
        ImGui::DragFloat3("Position", &light.pos.x, 0.01);
        ImGui::ColorEdit3("Color", &light.color.x);
        ImGui::DragFloat("Brightness", &light.bright, 0.01);
        ImGui::Button("Delete");
        if(ImGui::IsItemClicked())
          lights.erase(lights.begin() + i);
        ImGui::PopID();
      }

      ImGui::End();
    }

    if(cameraLight)
      pRenderer->AddPointLight(ne::PointLight(cameraPos, cameraLightCol, 1.0f));

    if(sun)
      pRenderer->AddDirectionalLight(ne::DirectionalLight(sunDir, sunCol, sunStrength));

    if(floatingLight)
      pRenderer->AddPointLight(ne::PointLight(lightPos, floatingLightCol, 1.0f));

    /* pRenderer->AddSpotLight( */
    /*       ne::SpotLight(cameraPos, cameraNorm, */
    /*       glm::radians(10.0f), */
    /*       glm::radians(25.0f), */
    /*       glm::vec3(1))); */

    /* pRenderer->AddSpotLight( */
    /*       ne::SpotLight( */
    /*         glm::vec3(11.916, 8.254, 5.693), */
    /*         glm::normalize(glm::vec3(-1.0, -0.2, -0.6)), */
    /*         glm::radians(20.0f), */
    /*         glm::radians(25.0f), */
    /*         glm::vec3(1), */
    /*         1.0f)); */

    /* pRenderer->AddSpotLight( */
    /*       ne::SpotLight( */
    /*         glm::vec3(4.5, 7.5, -6.0), */
    /*         glm::normalize(glm::vec3(0.0, -0.2, 1.0)), */
    /*         glm::radians(20.0f), */
    /*         glm::radians(25.0f), */
    /*         glm::vec3(1), */
    /*         1.0f)); */


    if(floatingLightSphere)
    {
      glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.1));
      glm::mat4 tran = glm::translate(glm::mat4(1.0), lightPos);
        pRenderer->AddDebugSphere(tran * scale, glm::vec3(1.0));
    }

    for(auto& light : lights)
    {
      if(light.enabled)
      {
        pRenderer->AddPointLight(ne::PointLight(light.pos, light.color, light.bright));
        glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(0.2));
        glm::mat4 tran = glm::translate(glm::mat4(1.0), light.pos);
        pRenderer->AddDebugSphere(tran * scale, glm::vec3(1.0));
      }
    }

    pRenderer->SetGlobalIllumination(glm::vec3(globalIllum));
    pRenderer->SetGamma(gamma);
    pRenderer->SetExposure(exposure);

    pRenderer->EndFrame();
    gui.Render();

    SDL_GL_SwapWindow(pWindow);
    SDL_Delay(10);

    //Handle events
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
      gui.HandleEvent(&e);

      if(gui.UsingMouse() && (
            e.type == SDL_MOUSEMOTION ||
            e.type == SDL_MOUSEBUTTONDOWN ||
            e.type == SDL_MOUSEBUTTONUP))
          continue;

      if(gui.UsingKeyboard() && (
            e.type == SDL_KEYDOWN ||
            e.type == SDL_KEYUP))
          continue;

      switch(e.type)
      {
        case SDL_QUIT:
          quit = true;
          break;

        case SDL_MOUSEMOTION:
          if(SDL_GetMouseState(NULL,NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
          {
            cameraYaw += 0.30 * dt * e.motion.xrel;
            cameraTilt += 0.30 * dt * e.motion.yrel;
            if(cameraTilt < glm::radians(-90.0f))
              cameraTilt = glm::radians(-90.0f);
            else if(cameraTilt > glm::radians(90.0f))
              cameraTilt = glm::radians(90.0f);
          }
          break;

        case SDL_MOUSEBUTTONDOWN:
          if(e.button.button == SDL_BUTTON_LEFT)
            SDL_SetRelativeMouseMode(SDL_TRUE);
          break;

        case SDL_MOUSEBUTTONUP:
          if(e.button.button == SDL_BUTTON_LEFT)
            SDL_SetRelativeMouseMode(SDL_FALSE);
          break;
      }
    }

    if(!gui.UsingKeyboard())
    {
      const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
      float moveSpeed = keyboard[SDL_SCANCODE_LSHIFT] ? 1 : 5;
      if(keyboard[SDL_SCANCODE_W])
        cameraPos += glm::vec3(moveSpeed * dt) * cameraNorm;
      if(keyboard[SDL_SCANCODE_S])
        cameraPos -= glm::vec3(moveSpeed * dt) * cameraNorm;
      if(keyboard[SDL_SCANCODE_A])
        cameraPos -= glm::vec3(moveSpeed * dt) * glm::normalize(glm::cross(cameraNorm,glm::vec3(0,1,0)));
      if(keyboard[SDL_SCANCODE_D])
        cameraPos += glm::vec3(moveSpeed * dt) * glm::normalize(glm::cross(cameraNorm,glm::vec3(0,1,0)));
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
  }

  delete pRenderer;

  SDL_GL_DeleteContext(GLcontext);
  SDL_DestroyWindow(pWindow);
  SDL_Quit();
  return 0;
}
