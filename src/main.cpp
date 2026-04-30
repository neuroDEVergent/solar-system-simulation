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
#include "shadowMapFramebuffer.h"

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));

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

  shadowFBO shadowMapFramebuffer = {0};
  shadowFBOInit(&shadowMapFramebuffer, 4096, 4096);

  // build and compile shaders
  Shader planetShader("./shaders/planet-vs.glsl", "./shaders/planet-fs.glsl");
  planetShader.use();
  planetShader.setInt("diffuseMap", 0);
  planetShader.setInt("shadowMap", 1);
  Shader postProcessShader("./shaders/post-process-vs.glsl", "./shaders/post-process-fs.glsl");
  Shader depthShader("./shaders/depth-vs.glsl","./shaders/depth-fs.glsl");

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

  float simSpeed = 0.0000;

  while (!win.quit)
  {

    float time = static_cast<float>(SDL_GetTicks() / 1000.0f);
    deltaTime = time - lastFrame;
    lastFrame = time;

    // Handle input 
    Input(&win, &camera, deltaTime);
    
    float simTime = time * simSpeed;

    // Light pass
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer.framebuffer);
    glViewport(0, 0, shadowMapFramebuffer.width, shadowMapFramebuffer.height);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthShader.use();

    glm::mat4 model;
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 400.0f);
    glm::mat4 lightView = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpace = lightProjection * lightView;
    depthShader.setMat4("lightSpace", lightSpace);

    // First planet
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(glm::sin(time), 0.0, -6.0f));
    depthShader.setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, planets[2].diffuseTexture);
    sphere.Draw(depthShader);
    // Second planet
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, 0.0, -12.0f));
    depthShader.setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, planets[3].diffuseTexture);
    sphere.Draw(depthShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Render pass

    // Draw the scene in multisampled buffers
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer.framebuffer);
    glViewport(0, 0, win.width, win.height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    // Declare matrices
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)win.width / (float)win.height, 0.1f, 400.0f);
    glm::mat4 view = camera.GetViewMatrix();
    model = glm::mat4(1.0f);
    glActiveTexture(GL_TEXTURE0);

   // Draw planets
    planetShader.use();
    planetShader.setMat4("projection", projection);
    planetShader.setMat4("view", view);
    planetShader.setMat4("lightSpace", lightSpace);
    planetShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    // First planet
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(glm::sin(time), 0.0, -6.0f));
    planetShader.setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planets[2].diffuseTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMapFramebuffer.shadowMap);
    sphere.Draw(planetShader);
    // Second planet
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0, 0.0, -12.0f));
    planetShader.setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planets[3].diffuseTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMapFramebuffer.shadowMap);
    sphere.Draw(planetShader);

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
