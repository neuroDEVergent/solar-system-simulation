#include <SDL.hpp>

static bool GLCheckErrorStatus(const char* function, int line)
{
  while (GLenum error = glGetError())
  {
    std::cout << "OpenGL Error:" << error
	      << "\tLine: " << line
	      << "\tfunction: " << function << std::endl;
    return true;
  }

  return false;
}
#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x, __LINE__);

void GetOpenGLVersionInfo()
{
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

bool InitializeSDL(
    Window*       window,
    unsigned int  width,
    unsigned int  height,
    const char*   title,
    bool          vsync
)
{

  if (!window) return false;

  window->width   = width;
  window->height  = height;
  window->title   = title;
  window->vsync   = vsync;
  window->quit    = false;
  
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    std::cout << "SDL2 could not initialize video subsystem" << std::endl;
    exit(1);
  }
  // Setup the OpenGL Context
  // Use OpenGL 4.1 core or greater
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  
  // We want to request a double buffer for smooth updating
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 2);

  // Create an application window using OpenGL that supports SDL
  window->sdlWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);

  // Check if Window did not create
  if (window->sdlWindow == nullptr)
  {
    std::cout << "SDL Window was not able to be created" << std::endl;
    exit(1);
  }

  // Create an OpenGL Graphics Context
  window->openGLContext = SDL_GL_CreateContext(window->sdlWindow);

  if (window->openGLContext == nullptr)
  {
    std::cout << "OpenGL context could not be created" << std::endl;
    exit(1);
  }

  // Initialize the Glad Library
  if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
  {
    std::cout << "glad was not initialized" << std::endl;
    exit(1);
  }

  GetOpenGLVersionInfo();
  
  return true;
}

void Input(Window* window, Camera* camera, float deltaTime)
{
  SDL_Event e;
  // Handle events on queue
  while(SDL_PollEvent(&e) != 0)
  {
    switch (e.type)
    {
      case SDL_QUIT:
      std::cout << "Goodbye!" << std::endl;
      window->quit = true;
      break;

      case SDL_MOUSEWHEEL:
        camera->ProcessMouseScroll(e.wheel.y);
        break;

      case SDL_MOUSEBUTTONDOWN:
        if (e.button.button == SDL_BUTTON_LEFT)
        {
          SDL_RaiseWindow(window->sdlWindow);
          SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        break;

      case SDL_MOUSEMOTION:
        float mouseX = (float) e.motion.xrel;
        float mouseY = (float) e.motion.yrel;
        camera->ProcessMouseMovement(mouseX, -mouseY);
        break;
      }

   }

  const Uint8* state = SDL_GetKeyboardState(NULL);
  if (state[SDL_SCANCODE_W]) camera->ProcessKeyboard(FORWARD, deltaTime);
  if (state[SDL_SCANCODE_S]) camera->ProcessKeyboard(BACKWARD, deltaTime);
  if (state[SDL_SCANCODE_D]) camera->ProcessKeyboard(RIGHT, deltaTime);
  if (state[SDL_SCANCODE_A]) camera->ProcessKeyboard(LEFT, deltaTime);
  if (state[SDL_SCANCODE_E]) camera->ProcessKeyboard(UP, deltaTime);
  if (state[SDL_SCANCODE_Q]) camera->ProcessKeyboard(DOWN, deltaTime);
  if (state[SDL_SCANCODE_K]) camera->exposure += 0.01f;
  if (state[SDL_SCANCODE_J]) camera->exposure -= 0.01;
  if (state[SDL_SCANCODE_LSHIFT]) camera->SPRINT = true;
  else camera->SPRINT = false;

}


void DestroySDL(Window* window)
{
  if (!window) return;
  
  if (window->openGLContext)
  {
    SDL_GL_DeleteContext(window->openGLContext);
    window->openGLContext = NULL;
  }
  
  if (window->sdlWindow)
  {
    SDL_DestroyWindow(window->sdlWindow);
    window->sdlWindow = NULL;
  }
 
  SDL_Quit();
}
