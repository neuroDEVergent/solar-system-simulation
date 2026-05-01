#ifndef SHADOWMAPFRAMEBUFFER_H
#define SHADOWMAPFRAMEBUFFER_H

#include <iostream>
#include <glad/glad.h>

typedef struct shadowMapFBO
{
  unsigned int framebuffer;
  unsigned int shadowMap;
  unsigned int width;
  unsigned int height;
  
} shadowMapFBO;

void shadowMapFBOInit(shadowMapFBO* fbo, unsigned int width, unsigned int height);

#endif
