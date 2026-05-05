#ifndef HDRSTORAGEBUFFER_H
#define HDRSTORAGEBUFFER_H

#include <glad/glad.h>
#include <iostream>

typedef struct hdrSSBO
{
  unsigned int framebuffer;
  unsigned int texture;
  unsigned int width;
  unsigned int height;
} hdrSSBO;

void hdrSSBOInit(hdrSSBO* fbo, unsigned int width, unsigned int height);

#endif
