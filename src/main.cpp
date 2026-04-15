/*
  Compilation on Linux
  g++ -std=c++17 ./src/* -o prog -I ./include/ -I ./thirdparty/glm-master/ -lSDL2 -ldl -lassimp
*/

// Third Party Libraries
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

// Personal libraries
#include <Shader.hpp>
#include <Camera.hpp>
#include <model.h>

// #################### vvv Globals vvv ####################
// Globals are prefixed with 'g'

// Screen dimensions
int gScreenWidth = 1920;
int gScreenHeight = 1080;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Main loop flag
bool gQuit = false; // If this is true then the program terminates

// #################### ^^^ Globals ^^^ ####################

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
// #################### ^^^ Error handling routines ^^^ ####################

void GetOpenGLVersionInfo()
{
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
  std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

unsigned int loadTexture(char const * path)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  //stbi_set_flip_vertically_on_load(true);
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1) format = GL_RED;
    else if (nrComponents == 3) format = GL_RGB;
    else if (nrComponents == 4) format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "failed to load texture" << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

void InitializeProgram()
{
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
  gGraphicsApplicationWindow = SDL_CreateWindow("OpenGL Window", 0, 0, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);

  // Check if Window did not create
  if (gGraphicsApplicationWindow == nullptr)
  {
    std::cout << "SDL Window was not able to be created" << std::endl;
    exit(1);
  }

  // Create an OpenGL Graphics Context
  gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);

  if (gOpenGLContext == nullptr)
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
}

void Input()
{
  SDL_Event e;
  // Handle events on queue
  while(SDL_PollEvent(&e) != 0)
  {
    switch (e.type)
    {
      case SDL_QUIT:
      std::cout << "Goodbye!" << std::endl;
      gQuit = true;
      break;

      case SDL_MOUSEWHEEL:
        camera.ProcessMouseScroll(e.wheel.y);
        break;

      case SDL_MOUSEBUTTONDOWN:
        if (e.button.button == SDL_BUTTON_LEFT)
        {
          SDL_RaiseWindow(gGraphicsApplicationWindow);
          SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        break;

      case SDL_MOUSEMOTION:
        float mouseX = (float) e.motion.xrel;
        float mouseY = (float) e.motion.yrel;
        camera.ProcessMouseMovement(mouseX, -mouseY);
        break;
      }

   }

  const Uint8* state = SDL_GetKeyboardState(NULL);
  if (state[SDL_SCANCODE_W]) camera.ProcessKeyboard(FORWARD, deltaTime);
  if (state[SDL_SCANCODE_S]) camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (state[SDL_SCANCODE_D]) camera.ProcessKeyboard(RIGHT, deltaTime);
  if (state[SDL_SCANCODE_A]) camera.ProcessKeyboard(LEFT, deltaTime);
  if (state[SDL_SCANCODE_E]) camera.ProcessKeyboard(UP, deltaTime);
  if (state[SDL_SCANCODE_Q]) camera.ProcessKeyboard(DOWN, deltaTime);

}

void CleanUp()
{
  SDL_DestroyWindow(gGraphicsApplicationWindow);
  SDL_Quit();
}

int main( int argc, char* args[] )
{
  
  // 1. Setup the graphics program
  InitializeProgram();

  glEnable(GL_DEPTH_TEST);

  // build and compile shaders
  Shader defaultShader("./shaders/default-vs.glsl", "./shaders/default-fs.glsl");

  // load models
  Model sphere("./resources/models/sphere.obj");
  unsigned int sunTexture = loadTexture("./resources/textures/sun-texture.jpg");


  
  while (!gQuit)
  {
    float time = static_cast<float>(SDL_GetTicks() / 1000.0f);
    deltaTime = time - lastFrame;
    lastFrame = time;

    // Handle input 
    Input();

    // Initialize clear color
    // This is the background of the screen
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // Clear color buffer and depth buffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
      
    // Use our shader
    defaultShader.use();

    // Projection matrix
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 100.0f);
    defaultShader.setMat4("projection", projection);
    defaultShader.setMat4("view", camera.GetViewMatrix());

    
    glm::mat4 model = glm::mat4(1.0f);
    defaultShader.setMat4("model", model);



    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sunTexture);
    sphere.Draw(defaultShader);

    SDL_GL_SwapWindow(gGraphicsApplicationWindow);
  }

  // 5. Call the cleanup funcion when our program terminates
  CleanUp();

  return 0;
  
}
