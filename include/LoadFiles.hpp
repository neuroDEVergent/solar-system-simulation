#ifndef LOADFILES_HPP
#define LOADFILES_HPP

#include <stb_image.h>
#include <iostream>
#include <glad/glad.h>
#include <vector>
#include <string>

unsigned int loadTexture(char const * path);
unsigned int loadCubemap(std::vector<std::string> faces);

#endif
