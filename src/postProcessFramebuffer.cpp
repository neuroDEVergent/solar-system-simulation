#include "postProcessFramebuffer.h"

void postProcessFBOInit(postProcessFBO* fbo, unsigned int width, unsigned int height)
{
  float quadVertices[] = {
  
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f, -1.0f, 1.0f, 0.0f,

    -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f
  };

  glGenVertexArrays(1, &fbo->VAO);
  glGenBuffers(1, &fbo->VBO);
  glBindVertexArray(fbo->VAO);
  glBindBuffer(GL_ARRAY_BUFFER, fbo->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

  glGenFramebuffers(1, &fbo->framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo->framebuffer);

  glGenTextures(1, &fbo->texture);
  glBindTexture(GL_TEXTURE_2D, fbo->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void msaaBindRead(GLenum TextureUnit);
