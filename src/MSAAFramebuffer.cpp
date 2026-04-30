#include "MSAAFramebuffer.h"

void msaaFBOInit(msaaFBO* fbo, unsigned int width, unsigned int height)
{
  glGenFramebuffers(1, &fbo->framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo->framebuffer);

  glGenTextures(1, &fbo->texture);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->texture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, width, height, GL_TRUE);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, fbo->texture, 0);

  glGenRenderbuffers(1, &fbo->rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo->rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//void msaaBindRead(GLenum TextureUnit);
