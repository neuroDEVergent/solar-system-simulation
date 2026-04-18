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

typedef struct Planet
{
  float realDiameter;
  float realDistance;
  float normalizedDiameter;
  float normalizedDistance;
  float realRotationSpeed;
  float normalizedRotationSpeed;
  float realOrbitalSpeed;
  float normalizedOrbitalSpeed;
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
   Rotation Speeds and Times (Equator):

    Mercury: 58d 16h (10.83 km/h)
    Venus: 243d 26m (6.52 km/h)
    Earth: 23h 56m (1574 km/h)
    Mars: 24h 36m (866 km/h)
    Jupiter: 9h 55m (45,583 km/h)
    Saturn: 10h 33m (36,840 km/h)
    Uranus: 17h 14m (14,794 km/h)
    Neptune: 16h (9,719 km/h)
  */

  /*
    Planet 		Orbital velocity[6]
    Mercury 	47.9 km/s (29.8 mi/s)
    Venus 	  35.0 km/s (21.7 mi/s)
    Earth 	  29.8 km/s (18.5 mi/s)
    Mars 	    24.1 km/s (15.0 mi/s)
    Jupiter 	13.1 km/s (8.1 mi/s)
    Saturn 	  9.7 km/s (6.0 mi/s)
    Uranus 	  6.8 km/s (4.2 mi/s)
    Neptune 	5.4 km/s (3.4 mi/s)
   */

  Planet sun = {0};
  sun.realDiameter = 1391400.f;
  sun.realDistance = 0.0f;
  sun.realRotationSpeed = 7189.0f;
  sun.realOrbitalSpeed = 0.0f;
  sun.texture = loadTexture("./resources/textures/planets/sun-texture.jpg");

  Planet mercury = {0};
  mercury.realDiameter = 4879.f;
  mercury.realDistance = 57900000.0f;
  mercury.realRotationSpeed = 10.83f;
  mercury.realOrbitalSpeed = 47.9f * 3600.0f;
  mercury.texture = loadTexture("./resources/textures/planets/mercury-texture.jpg");

  Planet venus = {0};
  venus.realDiameter = 12104.0f;
  venus.realDistance = 108200000.0f;
  venus.realRotationSpeed = 6.52f;
  venus.realOrbitalSpeed = 35.0f * 3600.0f;
  venus.texture = loadTexture("./resources/textures/planets/venus-texture.jpg");

  Planet earth = {0};
  earth.realDiameter = 12756.0f;
  earth.realDistance = 149600000.0f;
  earth.realRotationSpeed = 1574.0f;
  earth.realOrbitalSpeed = 29.8f * 3600.0f;
  earth.texture = loadTexture("./resources/textures/planets/earth-texture.jpg");

  Planet mars = {0};
  mars.realDiameter = 6792.0f;
  mars.realDistance = 227900000.0f;
  mars.realRotationSpeed = 866.0f;
  mars.realOrbitalSpeed = 24.1f * 3600.0f;
  mars.texture = loadTexture("./resources/textures/planets/mars-texture.jpg");

  Planet jupiter = {0};
  jupiter.realDiameter = 142984.0f;
  jupiter.realDistance = 778600000.0f;
  jupiter.realRotationSpeed = 45583.0f;
  jupiter.realOrbitalSpeed = 13.1f * 3600.0f;
  jupiter.texture = loadTexture("./resources/textures/planets/jupiter-texture.jpg");

  Planet saturn = {0};
  saturn.realDiameter = 120636.0f;
  saturn.realDistance = 1433500000.0f;
  saturn.realRotationSpeed = 36840.0f;
  saturn.realOrbitalSpeed = 9.7f * 3600.0f;
  saturn.texture = loadTexture("./resources/textures/planets/saturn-texture.jpg");

  Planet uranus = {0};
  uranus.realDiameter = 51118.0f;
  uranus.realDistance = 2872500000.0f;
  uranus.realRotationSpeed = 14794.0f;
  uranus.realOrbitalSpeed = 6.8f * 3600.0f;
  uranus.texture = loadTexture("./resources/textures/planets/uranus-texture.jpg");

  Planet neptune = {0};
  neptune.realDiameter = 49528.0f;
  neptune.realDistance = 4495100000.0f;
  neptune.realRotationSpeed = 9719.0f;
  neptune.realOrbitalSpeed = 5.4f * 3600.0f;
  neptune.texture = loadTexture("./resources/textures/planets/neptune-texture.jpg");

  Planet planets[] = {sun, mercury, venus, earth, mars, jupiter, saturn, uranus, neptune};

  for (unsigned int i = 0; i < std::size(planets); i++)
  {
    planets[i].normalizedDiameter = planets[i].realDiameter / 12756.0f;
    planets[i].normalizedDiameter = glm::pow(planets[i].normalizedDiameter, 0.5f);

    planets[i].normalizedDistance = planets[i].realDistance / 149600000.0f; 
    planets[i].normalizedDistance *= 1000.0f;
    planets[i].normalizedDistance = glm::pow(planets[i].normalizedDistance, 0.5f);

    planets[i].normalizedRotationSpeed = planets[i].realRotationSpeed;
//    planets[i].normalizedRotationSpeed = glm::pow(planets[i].normalizedRotationSpeed, 0.5f);
    
    planets[i].normalizedOrbitalSpeed = planets[i].realOrbitalSpeed;
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

    float simSpeed = 1.f;
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
    projection = glm::perspective(glm::radians(camera.Zoom), (float)gScreenWidth / (float)gScreenHeight, 0.1f, 200.0f);

    
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
      if (i != 0)
      {
        float angularVelocity = planets[i].realOrbitalSpeed / planets[i].realDistance;
        angle = angularVelocity * simTime;
      }

      else angle = simTime;

      float x = glm::cos(-angle) * planets[i].normalizedDistance;
      float z = glm::sin(-angle) * planets[i].normalizedDistance;

      model = glm::translate(model, glm::vec3(x, 0.0f, z));

      model = glm::scale(model, glm::vec3(planets[i].normalizedDiameter));
      model = glm::rotate(model, glm::radians(planets[i].normalizedRotationSpeed * simTime), glm::vec3(0.0f, 1.0f, 0.0f));
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

    SDL_GL_SwapWindow(gGraphicsApplicationWindow);
  }

  // 5. Call the cleanup funcion when our program terminates
  CleanUp();

  return 0;
  
}
