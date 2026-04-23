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

  float quadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
  // setup screen VAO
  unsigned int quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  // Configure MSAA framebuffer
  unsigned int msaaframebuffer;
  glGenFramebuffers(1, &msaaframebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, msaaframebuffer);
  // Create multisampled color attachment texture
  unsigned int textureColorBufferMultiSampled;
  glGenTextures(1, &textureColorBufferMultiSampled);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, win.width, win.height,GL_TRUE);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
  // Create renderbuffer object for depth and stencil attachments
  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, win.width, win.height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Configure second post-processing framebuffer
  unsigned int postprocessingframebuffer;
  glGenFramebuffers(1, &postprocessingframebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, postprocessingframebuffer);
  // Create color attachment
  unsigned int screenTexture;
  glGenTextures(1, &screenTexture);
  glBindTexture(GL_TEXTURE_2D, screenTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, win.width, win.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Screen Framebuffer is not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // build and compile shaders
  Shader sunShader("./shaders/sun-vs.glsl","./shaders/sun-fs.glsl");
  Shader planetShader("./shaders/planet-vs.glsl", "./shaders/planet-fs.glsl");
  Shader cubeMapShader("./shaders/cube-map-vs.glsl", "./shaders/cube-map-fs.glsl");
  Shader postProcessShader("./shaders/post-process-vs.glsl", "./shaders/post-process-fs.glsl");

  Shader earthShader("./shaders/planet-vs.glsl", "./shaders/earth-fs.glsl");
  earthShader.use();
  earthShader.setInt("defaultTexture", 0);
  earthShader.setInt("normalMap", 1);
  earthShader.setInt("specularMap", 2);
  earthShader.setInt("nightMap", 3);
  earthShader.setInt("clouds", 4);

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

  float simSpeed = 0.0;

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
    glBindFramebuffer(GL_FRAMEBUFFER, msaaframebuffer);
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
    glBindTexture(GL_TEXTURE_2D, planets[0].texture);
    sphere.Draw(sunShader);

    // Draw Earth
    earthShader.use();
    earthShader.setMat4("projection", projection);
    earthShader.setMat4("view", view);
    earthShader.setVec3("viewPos", camera.Position.x, camera.Position.y, camera.Position.z);
    model = glm::mat4(1.0f);

    float angle = simTime / planets[1].normalizedYear;
    float x = glm::cos(-angle) * planets[1].normalizedDistance;
    float z = glm::sin(-angle) * planets[1].normalizedDistance;
    model = glm::translate(model, glm::vec3(x, 0.0f, z));
    model = glm::scale(model, glm::vec3(planets[1].normalizedDiameter));
    model = glm::rotate(model, static_cast<float>(time * 0.01 * planets[1].normalizedDay), glm::vec3(0.0f, 1.0f, 0.0f));
    earthShader.setMat4("model", model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planets[1].texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, planets[1].normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, planets[1].specularMap);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, planets[1].nightMap);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, planets[1].clouds);
    sphere.Draw(earthShader);

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
      glBindTexture(GL_TEXTURE_2D, planets[i].texture);
      
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
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaframebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessingframebuffer);
    glBlitFramebuffer(0, 0, win.width, win.height, 0, 0, win.width, win.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Now render quad with scene's visuals as it's texture image
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    // Draw screen quad
    postProcessShader.use();
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    SDL_GL_SwapWindow(win.sdlWindow); 
    
  }

  // 5. Call the cleanup funcion when our program terminates
  DestroySDL(&win);

  return 0;
  
}
