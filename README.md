# 3D-Grapher

This is a visualization tool for graphing 3D functions, particularly useful for studying calculus III (originally developed for use in that class).  

Download the latest release [here](https://github.com/TheNumbat/3D-Grapher/releases).

### Features
Current
  - rectangular functions
  - cylindrical functions
  - spherical functions
  - parametric lines
  - visualization settings: lighting, wireframe, normalization, FOV, orbiting/movable camera
  - domain settings  
  
  Planned
  - examples
  - rectangular surfaces
  - parametric surface
  - vector fields 
  - partial derivative evaluation
  - double/triple integral evaluation
  - line/surface integral evaluation
  - curve highlighting

### Tools
  - C++11/STL
  - SDL2.0 for windowing, events, and OpenGL setup
  - SDL_ttf for font loading
  - GLM for vector/matrix math
  
### Technical Features
  - Custom retained GUI system (kind of messy)
  - Infix-to-prefix expression parsing
    - operator precedence, parenthesis, etc.
  - Stack-based prefix expression evaluation
  - Multithreaded graph generation
  - Basic lighting shaders

### Screenshots
![functions](https://raw.githubusercontent.com/TheNumbat/3D-Grapher/f0223d71813ce799c8a96d277f617e101f73ab94/Releases/Screenshots/Functions.png)
![settings](https://raw.githubusercontent.com/TheNumbat/3D-Grapher/f0223d71813ce799c8a96d277f617e101f73ab94/Releases/Screenshots/Settings.png)
