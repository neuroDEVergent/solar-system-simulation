#ifndef SDL_HPP
#define SDL_HPP

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <Camera.hpp>
#include <stdio.h>
#include <iostream>

typedef struct Window
{
  SDL_Window*   sdlWindow;
  SDL_GLContext openGLContext;
  
  unsigned int  width;
  unsigned int  height;
  const char*   title;
  
  bool          quit;
  bool          vsync;
} Window;

static bool GLCheckErrorStatus(const char* function, int line);
void GetOpenGLVersionInfo();

bool InitializeSDL(
    Window*       window,
    unsigned int  width,
    unsigned int  height,
    const char*   title,
    bool          vsync = true
);

void Input(Window* window, Camera* camera, float deltaTime);
void DestroySDL(Window* window);

#endif
