#ifndef MSAAFRAMEBUFFER
#define MSAAFRAMEBUFFER

#include <glad/glad.h>
#include <iostream>

typedef struct msaaFBO
{
  unsigned int framebuffer;
  unsigned int texture;
  unsigned int rbo;
} msaaFBO;

  
void msaaFBOInit(msaaFBO* fbo, unsigned int width, unsigned int height);
//void msaaFBOBindRead(GLenum TextureUnit);

#endif
