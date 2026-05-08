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
#include "ComputeShader.hpp"
#include "Camera.hpp"
#include "model.h"
#include "PlanetSpecification.hpp"
#include "LoadFiles.hpp"
#include "SDL.hpp"
#include "MSAAFramebuffer.h"
#include "postProcessFramebuffer.h"
#include "shadowMapFramebuffer.h"
#include "hdrFramebuffer.h"

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

  postProcessFBO extractFramebuffer = {0};
  postProcessFBOInit(&extractFramebuffer, win.width, win.height);

  shadowMapFBO shadowMapFramebuffer = {0};
  shadowMapFBOInit(&shadowMapFramebuffer, 4096, 4096);

  unsigned int luminanceSSBO = 0;

  unsigned int pingPongFBO[2];
  unsigned int pingPongColorBuffers[2];
  glGenFramebuffers(2, pingPongFBO);
  glGenTextures(2, pingPongColorBuffers);
  for (unsigned int i = 0; i < 2; i++)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[i]);
    glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, win.width, win.height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongColorBuffers[i], 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cout << "Framebuffer not complete" << std::endl;
  }

//  hdrFBO hdrFramebuffer = {0};
//  hdrFBOInit(&hdrFramebuffer, win.width, win.height);

  // build and compile shaders
  ComputeShader hdrShader("./shaders/hdr-cs.glsl");
  Shader sunShader("./shaders/sun-vs.glsl","./shaders/sun-fs.glsl");
  Shader planetShader("./shaders/planet-vs.glsl", "./shaders/planet-fs.glsl");
  planetShader.use();
  planetShader.setInt("diffuseMap", 0);
  planetShader.setInt("shadowMap", 1);
  Shader cubeMapShader("./shaders/cube-map-vs.glsl", "./shaders/cube-map-fs.glsl");
  Shader depthShader("./shaders/depth-vs.glsl", "./shaders/depth-fs.glsl", "./shaders/depth-gs.glsl");
  Shader postProcessShader("./shaders/post-process-vs.glsl", "./shaders/post-process-fs.glsl");
  postProcessShader.use();
  postProcessShader.setInt("screenTexture", 0);
  postProcessShader.setInt("bloomBlur", 1);
  Shader extractShader("./shaders/post-process-vs.glsl","./shaders/extract-fs.glsl");
  Shader earthShader("./shaders/earth-vs.glsl", "./shaders/earth-fs.glsl");
  earthShader.use();
  earthShader.setInt("diffuseMap", 0);
  earthShader.setInt("shadowMap", 1);
  earthShader.setInt("normalMap", 2);
  earthShader.setInt("specularMap", 3);
  earthShader.setInt("nightMap", 4);
  earthShader.setInt("cloudMap", 5);
  Shader blurShader("./shaders/blur-vs.glsl","./shaders/blur-fs.glsl");

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

  float simSpeed = 0.02;

  glm::vec3 lightPos = glm::vec3(0.0, 0.0, 0.0);
  glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
  camera.exposure = 1.0f;

  unsigned int localSizeX = 10;
  unsigned int localSizeY = 10;

  unsigned int numGroupsX = (win.width + localSizeX - 1) / localSizeX;
  unsigned int numGroupsY = (win.height + localSizeY - 1) / localSizeY;
  
  unsigned int numTiles = numGroupsX * numGroupsY;

  // Create SSBO
  glGenBuffers(1, &luminanceSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, luminanceSSBO);
  glBufferData(GL_SHADER_STORAGE_BUFFER, numTiles * sizeof(float), NULL, GL_DYNAMIC_COPY);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, luminanceSSBO);


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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer.framebuffer);
    glViewport(0, 0, shadowMapFramebuffer.width, shadowMapFramebuffer.height);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Create matrices
    float near_plane = 0.1f;
    float far_plane = 400.0f;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)shadowMapFramebuffer.width / (float)shadowMapFramebuffer.height, near_plane, far_plane);
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)));

    // Render to cubemap
    depthShader.use();
    for (unsigned int i = 0; i < 6; i++)
    {
      depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    }
    depthShader.setFloat("far_plane", far_plane);
    depthShader.setVec3("lightPos", lightPos);
    glm::mat4 model;

    for (unsigned int i = 1; i < std::size(planets); i++)
    {
      model = glm::mat4(1.0f);

      float angle = simTime / planets[i].normalizedYear;

      float x = glm::cos(-angle) * planets[i].normalizedDistance;
      float z = glm::sin(-angle) * planets[i].normalizedDistance;

      model = glm::translate(model, glm::vec3(x, 0.0f, z));

      model = glm::scale(model, glm::vec3(planets[i].normalizedDiameter));
      model = glm::rotate(model, static_cast<float>(simTime * planets[i].normalizedDay), glm::vec3(0.0f, 1.0f, 0.0f));

      depthShader.setMat4("model", model);
      glBindTexture(GL_TEXTURE_2D, planets[i].diffuseTexture);
      
      sphere.Draw(depthShader);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCullFace(GL_BACK);

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

    // Draw Sun
    sunShader.use();
    sunShader.setVec2("u_resolution", win.width, win.height);
    sunShader.setFloat("u_time", time);
    sunShader.setMat4("projection", projection);
    sunShader.setMat4("view", view);
    sunShader.setVec3("lightColor", lightColor);
    model = glm::scale(model, glm::vec3(planets[0].normalizedDiameter));
    model =  glm::rotate(model, simTime * static_cast<float>((planets[0].day / 24.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
    sunShader.setMat4("model", model);
    glBindTexture(GL_TEXTURE_2D, planets[0].diffuseTexture);
    sphere.Draw(sunShader);

    // Draw Earth
    earthShader.use();
    earthShader.setFloat("far_plane", far_plane);
    earthShader.setMat4("projection", projection);
    earthShader.setVec3("lightColor", lightColor);
    earthShader.setMat4("view", view);
    earthShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    earthShader.setVec3("lightPos", 0.0f, 0.0f, 0.0f);
    model = glm::mat4(1.0f);

    float angle = simTime / planets[1].normalizedYear;
    float x = glm::cos(-angle) * planets[1].normalizedDistance;
    float z = glm::sin(-angle) * planets[1].normalizedDistance;
    model = glm::translate(model, glm::vec3(x, 0.0f, z));
    model = glm::scale(model, glm::vec3(planets[1].normalizedDiameter));
    model = glm::rotate(model, static_cast<float>(simTime * planets[1].normalizedDay), glm::vec3(0.0f, 1.0f, 0.0f));
    earthShader.setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planets[1].diffuseTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMapFramebuffer.shadowMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, planets[1].normalMap);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, planets[1].specularMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, planets[1].nightMap);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, planets[1].clouds);
    sphere.Draw(earthShader);
   // Draw planets
    planetShader.use();
    planetShader.setMat4("projection", projection);
    planetShader.setMat4("view", view);
    planetShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    planetShader.setVec3("lightPos", lightPos);
    planetShader.setVec3("lightColor", lightColor);
    planetShader.setFloat("far_plane", far_plane);
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
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, planets[i].diffuseTexture);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMapFramebuffer.shadowMap);
      
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

    glBindFramebuffer(GL_FRAMEBUFFER, extractFramebuffer.framebuffer);
    extractShader.use();
    glBindTexture(GL_TEXTURE_2D, postProcessFramebuffer.texture);
    glBindVertexArray(postProcessFramebuffer.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Blur bright fragments with two pass Gaussian Blur
    bool horizontal = true, first_iteration = true;
    unsigned int amount = 5;
    blurShader.use();
    for (unsigned int i = 0; i < amount; i++)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[horizontal]);
      blurShader.setInt("horizontal", horizontal);
      glBindTexture(GL_TEXTURE_2D, first_iteration ? extractFramebuffer.texture : pingPongColorBuffers[!horizontal]);
      glBindVertexArray(postProcessFramebuffer.VAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      horizontal = !horizontal;
      if (first_iteration) first_iteration = false;
      glBindVertexArray(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Calculate average luminance
    hdrShader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessFramebuffer.texture);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, luminanceSSBO);

    glDispatchCompute(numGroupsX, numGroupsY, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, luminanceSSBO);
    float* data = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    if (!data) return 1.0f;

    float targetExposure;
    float biggest = 0.0f;
    for (int i = 0; i < numTiles; ++i)
    {
      if (data[i] >= biggest) biggest = data[i];
    }

    if (biggest >= 100.0) targetExposure = 0.15f;
    else targetExposure = 2.2;

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


    float speed = 1.3f;
    camera.exposure += (targetExposure - camera.exposure) * speed * deltaTime;

    // Now render quad with scene's visuals as it's texture image
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Draw screen quad
    postProcessShader.use();
    postProcessShader.setFloat("exposure", camera.exposure);
    glBindVertexArray(postProcessFramebuffer.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessFramebuffer.texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingPongColorBuffers[!horizontal]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    SDL_GL_SwapWindow(win.sdlWindow); 
    
  }

  // 5. Call the cleanup funcion when our program terminates
  DestroySDL(&win);

  return 0;
  
}
