#ifndef SHADOWMAPFRAMEBUFFER_H
#define SHADOWMAPFRAMEBUFFER_H

#include <iostream>
#include <glad/glad.h>

typedef struct shadowFBO
{
  unsigned int framebuffer;
  unsigned int shadowMap;
  unsigned int width;
  unsigned int height;
} shadowFBO;

void shadowFBOInit(shadowFBO* fbo, unsigned int width, unsigned int height);

#endif
