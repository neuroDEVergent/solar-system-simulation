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
#include "Shader.hpp"
#include "Camera.hpp"
#include "model.h"
#include "PlanetSpecification.hpp"
#include "LoadFiles.hpp"
#include "SDL.hpp"
#include "MSAAFramebuffer.h"
#include "postProcessFramebuffer.h"

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

  msaaFBO msaaFramebuffer = {0};
  msaaFBOInit(&msaaFramebuffer, win.width, win.height);

  postProcessFBO postProcessFramebuffer = {0};
  postProcessFBOInit(&postProcessFramebuffer, win.width, win.height);

  // build and compile shaders
  Shader sunShader("./shaders/sun-vs.glsl","./shaders/sun-fs.glsl");
  Shader planetShader("./shaders/planet-vs.glsl", "./shaders/planet-fs.glsl");
  Shader cubeMapShader("./shaders/cube-map-vs.glsl", "./shaders/cube-map-fs.glsl");
  Shader postProcessShader("./shaders/post-process-vs.glsl", "./shaders/post-process-fs.glsl");
  Shader earthShader("./shaders/earth-vs.glsl", "./shaders/earth-fs.glsl");
  Shader cloudShader("./shaders/planet-vs.glsl", "./shaders/clouds-fs.glsl");
  earthShader.use();
  earthShader.setInt("diffuseMap", 0);
  earthShader.setInt("normalMap", 1);
  earthShader.setInt("specularMap", 2);
  earthShader.setInt("nightMap", 3);

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

  float simSpeed = 0.0001;

  while (!win.quit)
  {

    float time = static_cast<float>(SDL_GetTicks() / 1000.0f);
    deltaTime = time - lastFrame;
    lastFrame = time;

    // Handle input 
    Input(&win, &camera, deltaTime);
    
    float simTime = time * simSpeed;

    glViewport(0, 0, win.width, win.height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    // Draw the scene in multisampled buffers
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer.framebuffer);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Declare matrices
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)win.width / (float)win.height, 0.1f, 400.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    glActiveTexture(GL_TEXTURE0);

    // Draw Sun
    sunShader.use();
    sunShader.setVec2("u_resolution", win.width, win.height);
    sunShader.setFloat("u_time", time);
    sunShader.setMat4("projection", projection);
    sunShader.setMat4("view", view);
    model = glm::scale(model, glm::vec3(planets[0].normalizedDiameter));
    model =  glm::rotate(model, simTime * static_cast<float>((planets[0].day / 24.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
    sunShader.setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, planets[0].diffuseTexture);
    sphere.Draw(sunShader);

    // Draw Earth
    earthShader.use();
    earthShader.setMat4("projection", projection);
    earthShader.setMat4("view", view);
    earthShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    earthShader.setVec3("lightPos", 0.0f, 0.0f, 0.0f);
    model = glm::mat4(1.0f);

    float angle = simTime / planets[1].normalizedYear * 0.0;
    float x = glm::cos(-angle) * planets[1].normalizedDistance;
    float z = glm::sin(-angle) * planets[1].normalizedDistance;
    model = glm::translate(model, glm::vec3(x, 0.0f, z));
    model = glm::scale(model, glm::vec3(planets[1].normalizedDiameter));
    model = glm::rotate(model, static_cast<float>(simTime * planets[1].normalizedDay), glm::vec3(0.0f, 1.0f, 0.0f));
    earthShader.setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planets[1].diffuseTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, planets[1].normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, planets[1].specularMap);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, planets[1].nightMap);
    sphere.Draw(earthShader);
    // Draw clouds
    cloudShader.use();
    cloudShader.setFloat("u_time", time);
    cloudShader.setMat4("projection", projection);
    cloudShader.setMat4("view", view);
    cloudShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, 0.0f, z));
    model = glm::scale(model, glm::vec3(planets[1].normalizedDiameter) * 1.005f);
    model = glm::rotate(model, static_cast<float>(simTime * planets[1].normalizedDay * 0.9), glm::vec3(0.0f, 1.0f, 0.0f));
    cloudShader.setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planets[1].clouds);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    sphere.Draw(cloudShader);
    // Draw planets
    planetShader.use();
    planetShader.setMat4("projection", projection);
    planetShader.setMat4("view", view);
    planetShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    for (unsigned int i = 2; i < std::size(planets); i++)
    {
      model = glm::mat4(1.0f);

      float angle;

      if (i == 0) angle = 0;
      else angle = simTime / planets[i].normalizedYear;

      float x = glm::cos(-angle) * planets[i].normalizedDistance;
      float z = glm::sin(-angle) * planets[i].normalizedDistance;

      model = glm::translate(model, glm::vec3(x, 0.0f, z));

      model = glm::scale(model, glm::vec3(planets[i].normalizedDiameter));
      model = glm::rotate(model, static_cast<float>(simTime * planets[i].normalizedDay), glm::vec3(0.0f, 1.0f, 0.0f));

      planetShader.setMat4("model", model);
      glBindTexture(GL_TEXTURE_2D, planets[i].diffuseTexture);
      
      sphere.Draw(planetShader);
    }

    // Draw stars cubemap
    glDepthFunc(GL_LEQUAL);
    cubeMapShader.use();
    cubeMapShader.setMat4("projection", projection);
    cubeMapShader.setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
    skyboxCube.Draw(cubeMapShader);
    glDepthFunc(GL_LESS);

    // Now blit multisampled buffer to normal colorbuffer of intermediate FBO
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFramebuffer.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postProcessFramebuffer.framebuffer);
    glBlitFramebuffer(0, 0, win.width, win.height, 0, 0, win.width, win.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Now render quad with scene's visuals as it's texture image
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Draw screen quad
    postProcessShader.use();
    glBindVertexArray(postProcessFramebuffer.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessFramebuffer.texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    SDL_GL_SwapWindow(win.sdlWindow); 
    
  }

  // 5. Call the cleanup funcion when our program terminates
  DestroySDL(&win);

  return 0;
  
}
