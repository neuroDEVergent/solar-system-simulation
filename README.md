# Solar System Simulation
Starting this project to practice my OpenGL and C/C++ skills. 

![SSSS](./resources/ssss.png)

## Dependencies
- GLAD
- SDL2
- Assimp
- GLM
- stb_image

## Build
Project is written on a GNU/Linux system. You can probably get it running on Windows with relative ease as well, but I'm not willing to be bothered with that. 

You can compile it with this simple command:
```bash
g++ -std=c++17 ./src/* \
-o prog \
-I ./include/ \
-I ./thirdparty/glad/ \
-I ./thirdparty/KHR/ \
-I ./thirdparty/glm-master/ \
-I ./thirdparty/ \
-lSDL2 \
-ldl \
-lassimp
```

And then simply run it by using:
```bash
./prog
```

## Resources I used
- [Solar System Wiki Page](https://en.wikipedia.org/wiki/Solar_System)
- [Make a Scale Solar System](https://www.jpl.nasa.gov/edu/resources/project/make-a-scale-solar-system/)
- [learnopengl.com](https://learnopengl.com/)
- [OpenGL Reference Pages](https://registry.khronos.org/OpenGL-Refpages/gl4/)
- [Solar System Textures](https://www.solarsystemscope.com/textures/)
- [HDRI to CubeMap convertor](https://matheowis.github.io/HDRI-to-CubeMap/)

## Cheklist
Below is the checklist of things I want to implement:

### Basics Graphics
- [x] ~Third-party libraries setup~
- [x] ~Shader abstraction~
- [x] ~Transformations~
- [x] ~Camera movement~
- [x] ~Model loading~

### Intermediate Graphics
- [x] ~Blinn-Phong lighting~
- [ ] Sun's fragment shader
- [x] ~Cubemap~
- [ ] Anti-aliasing
- [ ] Normal and depth maps
- [ ] Framebuffer for post-processing

### Advanced Graphics
- [ ] Bloom
- [ ] Shadows
- [ ] HDR
- [ ] Gamma handling
- [ ] SSAO

### Physics
_throwing these out the window, planets differ in size way too much and are way to far away for the project to actually look good_
- ~[ ] Correct planet scaling (possibly smaller Sun to fit the screen)~
- ~[ ] Correct distance between bodies~
_I actually implemeted them by using correct sizes, normalizing to Earth data, and mapping to a logarithmic function prevent large values from being too big_

- [x] ~Planets orbiting around the Sun~
- [ ] Correct time conversion with ability to speed time up

### Chef's Kiss
- [ ] "Photo mode" style UI elements to control post-processing shaders
- [ ] Audio player


![GifExample](./resources/eg.gif)
