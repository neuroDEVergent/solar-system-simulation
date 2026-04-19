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
#include <glm/gtc/constants.hpp>
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

typedef struct Planet
{
  float realDiameter;
  float realDistance;
  float normalizedDiameter;
  float normalizedDistance;
  float day;
  float normalizedDay;
  float year;
  float normalizedYear;
  unsigned int texture;
} Planet;

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

unsigned int loadCubemap(std::vector<std::string> faces)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++)
  {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    }

    else
    {
      std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
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
  if (state[SDL_SCANCODE_LSHIFT]) camera.SPRINT = true;
  else camera.SPRINT = false;

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
  Shader cubeMapShader("./shaders/cube-map-vs.glsl", "./shaders/cube-map-fs.glsl");

  /* NASA's reference guide
  Planet    Diameter (km) Distance from Sun (km)
  Sun       1,391,400     -
  Mercury   4,879         57,900,000
  Venus     12,104        108,200,000
  Earth     12,756        149,600,000
  Mars      6,792         227,900,000
  Jupiter   142,984       778,600,000
  Saturn    120,536       1,433,500,000
  Uranus    51,118        2,872,500,000
  Neptune   49,528        4,495,100,000
  */
  
  /*
    Day       Length
    Sun       648 hours
    Mercury 	1,408 hours
    Venus 	  5,832 hours
    Earth 	  24 hours
    Mars 	    25 hours
    Jupiter 	10 hours
    Saturn 	  11 hours
    Uranus 	  17 hours
    Neptune 	16 hours 
  */

  /*
   Year     Length
   Sun      0
   Mercury  88 days
   Venus    225 days
   Earth    365 days
   Mars     687 days
   Jupiter  4333 days
   Saturn   10759 days
   Uranus   30687 days
   Neptune  60190 days
  */

  Planet sun = {0};
  sun.realDiameter = 1391400.f;
  sun.realDistance = 0.0f;
  sun.day = 840.0f;
  sun.year = 0.0f;
  sun.texture = loadTexture("./resources/textures/planets/sun-texture.jpg");

  Planet mercury = {0};
  mercury.realDiameter = 4879.f;
  mercury.realDistance = 57900000.0f;
  mercury.day = 1408.0f;
  mercury.year = 88.0f;
  mercury.texture = loadTexture("./resources/textures/planets/mercury-texture.jpg");

  Planet venus = {0};
  venus.realDiameter = 12104.0f;
  venus.realDistance = 108200000.0f;
  venus.day = 5832.0f;
  venus.year = 225.0f;
  venus.texture = loadTexture("./resources/textures/planets/venus-texture.jpg");

  Planet earth = {0};
  earth.realDiameter = 12756.0f;
  earth.realDistance = 149600000.0f;
  earth.day = 24.0f;
  earth.year = 365.0f;
  earth.texture = loadTexture("./resources/textures/planets/earth-texture.jpg");

  Planet mars = {0};
  mars.realDiameter = 6792.0f;
  mars.realDistance = 227900000.0f;
  mars.day = 25.0f;
  mars.year = 687.0f;
  mars.texture = loadTexture("./resources/textures/planets/mars-texture.jpg");

  Planet jupiter = {0};
  jupiter.realDiameter = 142984.0f;
  jupiter.realDistance = 778600000.0f;
  jupiter.day = 10.0f;
  jupiter.year = 4333.0f;
  jupiter.texture = loadTexture("./resources/textures/planets/jupiter-texture.jpg");

  Planet saturn = {0};
  saturn.realDiameter = 120636.0f;
  saturn.realDistance = 1433500000.0f;
  saturn.day = 11.0f;
  saturn.year = 10759.0f;
  saturn.texture = loadTexture("./resources/textures/planets/saturn-texture.jpg");

  Planet uranus = {0};
  uranus.realDiameter = 51118.0f;
  uranus.realDistance = 2872500000.0f;
  uranus.day = 17.0f;
  uranus.year = 30687.0f;
  uranus.texture = loadTexture("./resources/textures/planets/uranus-texture.jpg");

  Planet neptune = {0};
  neptune.realDiameter = 49528.0f;
  neptune.realDistance = 4495100000.0f;
  neptune.day = 16.0f;
  neptune.year = 60190.0f;
  neptune.texture = loadTexture("./resources/textures/planets/neptune-texture.jpg");

  Planet planets[] = {sun, mercury, venus, earth, mars, jupiter, saturn, uranus, neptune};

  for (unsigned int i = 0; i < std::size(planets); i++)
  {
    planets[i].normalizedDiameter = planets[i].realDiameter / 12756.0f;
    planets[i].normalizedDiameter = glm::pow(planets[i].normalizedDiameter, 0.5f);

    planets[i].normalizedDistance = planets[i].realDistance / 149600000.0f; 
    planets[i].normalizedDistance *= 1000.0f;
    planets[i].normalizedDistance = glm::pow(planets[i].normalizedDistance, 0.5f);

    planets[i].normalizedYear = planets[i].year / 365.0f;
    planets[i].normalizedDay = planets[i].day / 24.0f;
    planets[i].normalizedDay = planets[i].year / planets[i].normalizedDay * planets[i].normalizedYear;
  }

 
  // load models
  Model sphere("./resources/models/sphere.obj");

  // Cubemap
  Model skyboxCube("./resources/models/cube.obj");
  std::vector<std::string> faces
  {
      "resources/textures/stars-cubemap/px.png",
      "resources/textures/stars-cubemap/nx.png",
      "resources/textures/stars-cubemap/py.png",
      "resources/textures/stars-cubemap/ny.png",
      "resources/textures/stars-cubemap/pz.png",
      "resources/textures/stars-cubemap/nz.png"
  };

  unsigned int spaceCubemap = loadCubemap(faces);

  while (!gQuit)
  {

    float time = static_cast<float>(SDL_GetTicks() / 1000.0f);
    deltaTime = time - lastFrame;
    lastFrame = time;

    float simSpeed = 0.2f;
    float simTime = time * simSpeed;

    // Handle input 
    Input();

    // Initialize clear color
    // This is the background of the screen
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // Clear color buffer and depth buffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 400.0f);

    
    // Use our shader
    defaultShader.use();
    defaultShader.setMat4("projection", projection);
    defaultShader.setMat4("view", camera.GetViewMatrix());

    glActiveTexture(GL_TEXTURE0);

    glm::mat4 model;

    for (unsigned int i = 0; i < std::size(planets); i++)
    {
      model = glm::mat4(1.0f);

      float angle;

      if (i == 0) angle = 0;
      else angle = simTime / planets[i].normalizedYear;

      float x = glm::cos(-angle) * planets[i].normalizedDistance;
      float z = glm::sin(-angle) * planets[i].normalizedDistance;

      model = glm::translate(model, glm::vec3(x, 0.0f, z));

      model = glm::scale(model, glm::vec3(planets[i].normalizedDiameter));
      if (i == 0) model =  glm::rotate(model, simTime * (planets[i].day / 24.0f), glm::vec3(0.0f, 1.0f, 0.0f));
      else model =  glm::rotate(model, simTime * planets[i].normalizedDay, glm::vec3(0.0f, 1.0f, 0.0f));

      if (i == 0)
      {
        std::cout << "SUN DAY: " << planets[i].day / 24.0f << std::endl;
      }
      defaultShader.setMat4("model", model);
      glBindTexture(GL_TEXTURE_2D, planets[i].texture);
      sphere.Draw(defaultShader);
    }

    // Draw stars cubemap
    glDepthFunc(GL_LEQUAL);
    cubeMapShader.use();
    cubeMapShader.setMat4("projection", projection);
    cubeMapShader.setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
    skyboxCube.Draw(cubeMapShader);
    glDepthFunc(GL_LESS);

    SDL_GL_SwapWindow(gGraphicsApplicationWindow); }

  // 5. Call the cleanup funcion when our program terminates
  CleanUp();

  return 0;
  
}
