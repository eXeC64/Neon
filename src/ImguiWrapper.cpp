#include "ImguiWrapper.hpp"

#include <SDL2/SDL.h>
#include <imgui.h>

namespace ne
{

  ImguiWrapper::ImguiWrapper()
  {
    m_time = 0.0;
    m_mousePressed[0] = false;
    m_mousePressed[1] = false;
    m_mousePressed[2] = false;
    m_mouseWheel = 0.0f;

    ImGuiIO& io = ImGui::GetIO();

    // Set up key map
    {
      io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
      io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
      io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
      io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
      io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
      io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
      io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
      io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
      io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
      io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
      io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
      io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
      io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
      io.KeyMap[ImGuiKey_A] = SDLK_a;
      io.KeyMap[ImGuiKey_C] = SDLK_c;
      io.KeyMap[ImGuiKey_V] = SDLK_v;
      io.KeyMap[ImGuiKey_X] = SDLK_x;
      io.KeyMap[ImGuiKey_Y] = SDLK_y;
      io.KeyMap[ImGuiKey_Z] = SDLK_z;
    }

    glGenBuffers(1, &m_vboVertices);
    glGenBuffers(1, &m_vboIndices);

    // Set up shader for the gui
    {
      const GLchar *vertexSrc = 
        "#version 300 es\n"
        "layout (location = 0) in vec2 vertexPos;\n"
        "layout (location = 1) in vec2 vertexUV;\n"
        "layout (location = 2) in vec4 vertexColor;\n"

        "out vec2 inUV;\n"
        "out vec4 inColor;\n"

        "uniform mat4 matView;\n"

        "void main()\n"
        "{\n"
        "   inUV = vertexUV;\n"
        "   inColor = vertexColor;\n"
        "   gl_Position = matView * vec4(vertexPos, 0, 1);\n"
        "}\n";

      const GLchar *fragSrc = 
        "#version 300 es\n"
        "precision highp float;\n"

        "in vec2 inUV;\n"
        "in vec4 inColor;\n"

        "uniform sampler2D texture;\n"

        "void main()\n"
        "{\n"
        "    gl_FragColor = inColor * texture2D(texture, inUV);\n"
        "}\n";

      GLuint vertHandle = glCreateShader(GL_VERTEX_SHADER);
      GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
      m_shader = glCreateProgram();

      glShaderSource(vertHandle, 1, &vertexSrc, 0);
      glCompileShader(vertHandle);

      glShaderSource(fragHandle, 1, &fragSrc, 0);
      glCompileShader(fragHandle);

      glAttachShader(m_shader, vertHandle);
      glAttachShader(m_shader, fragHandle);
      glLinkProgram(m_shader);
    }

    // Set up texture
    {
      unsigned char* pixels;
      int width, height;
      io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height); 
      glGenTextures(1, &m_texFont);
      glBindTexture(GL_TEXTURE_2D, m_texFont);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
      io.Fonts->TexID = (void *)(intptr_t)m_texFont;

      //Cleanup
      io.Fonts->ClearInputData();
      io.Fonts->ClearTexData();
    }
  }

  ImguiWrapper::~ImguiWrapper()
  {
    glDeleteProgram(m_shader);
    glDeleteTextures(1, &m_texFont);
    glDeleteBuffers(1, &m_vboVertices);
    glDeleteBuffers(1, &m_vboIndices);
  }

  void ImguiWrapper::NewFrame(SDL_Window *window)
  {
    ImGuiIO& io = ImGui::GetIO();

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    io.DisplaySize = ImVec2(w, h);

    double curTime = SDL_GetTicks() / 1000.0;
    io.DeltaTime = m_time > 0.0 ? curTime - m_time : 1.0f/60.0f;
    m_time = curTime;

    int mx, my;
    int mouseMask = SDL_GetMouseState(&mx, &my);
    if(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
      io.MousePos = ImVec2(mx, my);
    else
      io.MousePos = ImVec2(-1,-1);

    io.MouseDown[0] = m_mousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
    io.MouseDown[1] = m_mousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = m_mousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    m_mousePressed[0] = m_mousePressed[1] = m_mousePressed[2] = false;

    io.MouseWheel = m_mouseWheel;
    m_mouseWheel = 0.0f;

    //Hide mouse if imgui is drawing it
    SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

    ImGui::NewFrame();
  }

  void ImguiWrapper::Render()
  {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Render();
    ImDrawData *drawData = ImGui::GetDrawData();

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    glViewport(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    const float frameBufferHeight = io.DisplaySize.y * io.DisplayFramebufferScale.y;
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    const float ortho_projection[4][4] =
    {
      { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
      { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
      { 0.0f,                  0.0f,                  -1.0f, 0.0f },
      {-1.0f,                  1.0f,                   0.0f, 1.0f },
    };

    glUseProgram(m_shader);
    glUniform1i(glGetUniformLocation(m_shader, "texture"), 0);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "matView"), 1, GL_FALSE, &ortho_projection[0][0]);

    // Render command lists
    glBindBuffer(GL_ARRAY_BUFFER, m_vboVertices);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, pos));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, uv));
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)offsetof(ImDrawVert, col));

    for(int n = 0; n < drawData->CmdListsCount; n++)
    {
      const ImDrawList* cmd_list = drawData->CmdLists[n];
      const ImDrawIdx* idx_buffer_offset = 0;

      glBindBuffer(GL_ARRAY_BUFFER, m_vboVertices);
      glBufferData(GL_ARRAY_BUFFER, cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), &cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vboIndices);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), &cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

      for(const ImDrawCmd* cmd = cmd_list->CmdBuffer.begin(); cmd != cmd_list->CmdBuffer.end(); cmd++)
      {
        if (cmd->UserCallback)
        {
          cmd->UserCallback(cmd_list, cmd);
        }
        else
        {
          glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)cmd->TextureId);
          glScissor(cmd->ClipRect.x,
                    frameBufferHeight - cmd->ClipRect.w,
                    cmd->ClipRect.z - cmd->ClipRect.x,
                    cmd->ClipRect.w - cmd->ClipRect.y);
          glDrawElements(GL_TRIANGLES, cmd->ElemCount, GL_UNSIGNED_SHORT, idx_buffer_offset);
        }
        idx_buffer_offset += cmd->ElemCount;
      }
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
  }

  void ImguiWrapper::HandleEvent(const SDL_Event *event)
  {
    ImGuiIO& io = ImGui::GetIO();

    switch(event->type)
    {
      case SDL_MOUSEWHEEL:
        if(event->wheel.y > 0)
          m_mouseWheel = 1;
        else if(event->wheel.y < 0)
          m_mouseWheel = -1;
        break;

      case SDL_MOUSEBUTTONDOWN:
        if (event->button.button == SDL_BUTTON_LEFT)
          m_mousePressed[0] = true;
        else if (event->button.button == SDL_BUTTON_RIGHT)
          m_mousePressed[1] = true;
        else if (event->button.button == SDL_BUTTON_MIDDLE)
          m_mousePressed[2] = true;
        break;

      case SDL_TEXTINPUT:
        io.AddInputCharactersUTF8(event->text.text);
        break;

      case SDL_KEYDOWN:
      case SDL_KEYUP:
        int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
        io.KeysDown[key] = (event->type == SDL_KEYDOWN);
        io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
        io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
        io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
        break;
    }
  }

  bool ImguiWrapper::UsingMouse()
  {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
  }

  bool ImguiWrapper::UsingKeyboard()
  {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
  }
}
