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
#include <PlanetSpecification.hpp>
#include <LoadFiles.hpp>
#include <SDL.hpp>

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 20.0f));

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// #################### ^^^ Globals ^^^ ####################

int main( int argc, char* args[] )
{
  
  // 1. Setup the graphics program
  Window win = {0};
  if (!InitializeSDL(&win, 1920, 1080, "Solar System", true)) return 1;

  glEnable(GL_DEPTH_TEST);

  // build and compile shaders
  Shader defaultShader("./shaders/default-vs.glsl", "./shaders/default-fs.glsl");
  Shader cubeMapShader("./shaders/cube-map-vs.glsl", "./shaders/cube-map-fs.glsl");
  
  Planet planets[9];
  initializePlanets(planets, 9);
 
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
  
  float simSpeed = 0.2;

  while (!win.quit)
  {

    float time = static_cast<float>(SDL_GetTicks() / 1000.0f);
    deltaTime = time - lastFrame;
    lastFrame = time;


    // Handle input 
    Input(&win, &camera, deltaTime);
    
    float simTime = time * simSpeed;
    

    // Initialize clear color
    // This is the background of the screen
    glViewport(0, 0, win.width, win.height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    // Clear color buffer and depth buffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)win.width / (float)win.height, 0.1f, 400.0f);

    
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

    SDL_GL_SwapWindow(win.sdlWindow); 
    
  }

  // 5. Call the cleanup funcion when our program terminates
  DestroySDL(&win);

  return 0;
  
}
