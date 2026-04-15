# Solar System Simulation
Starting this project to practice my OpenGL and C/C++ skills. 

## Build
Project is written on a GNU/Linux system. You can probably get it running on Windows with relative ease as well, but I'm not willing to be bothered with that. 

You can compile it with this simple command:
```bash
g++ -std=c++17 ./src/* -o prog -I ./include/ -I ./thirdparty/glm-master/ -lSDL2 -ldl -lassimp
```

And then simply run it by using:
```bash
./prog
```

## Cheklist

Below is the checklist of things I want to implement:

### Basics Graphics
- [x] Third-party libraries setup
- [x] Shader abstraction
- [x] Transformations
- [x] Camera movement
- [x] Model loading

### Intermediate Graphics
- [ ] Blinn-Phong lighting
- [ ] Cubemap
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
- [ ] Correct planet scaling (possibly smaller Sun to fit the screen)
- [ ] Correct distance between bodies
- [ ] Planets orbiting around the Sun
- [ ] Correct time conversion with ability to speed time up

### Chef's Kiss
- [ ] "Photo mode" style UI elements to control post-processing shaders


