#ifndef POSTPRCESSFRAMEBUFFER_H
#define POSTPROCESSFRAMEBUFFER_H

#include <glad/glad.h>
#include <iostream>

typedef struct postProcessFBO
{
  unsigned int framebuffer;
  unsigned int texture;
  unsigned int VAO;
  unsigned int VBO;
} postProcessFBO;

  
void postProcessFBOInit(postProcessFBO* fbo, unsigned int width, unsigned int height);
//void msaaFBOBindRead(GLenum TextureUnit);

#endif
