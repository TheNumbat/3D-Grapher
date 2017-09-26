# 3D-Grapher

This is a visualization tool for graphing 3D functions, particularly useful for studying calculus III (originally developed for use in that class).  

Download the latest release [here](https://github.com/TheNumbat/3D-Grapher/releases).  

The code is rather messy...I make no claims as to correctness. Will eventually refactor to be more like [exile](https://github.com/TheNumbat/exile).  

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
  - level curves
  - rectangular surfaces
  - parametric surfaces
  - vector fields 
  - partial derivative evaluation
  - double/triple integral evaluation
  - line/surface integral evaluation
  - curve & intersection highlighting
  - animated transitions between graph states

### Tools
  - C++11/STL
  - SDL2.0 for windowing, events, and OpenGL setup
  - GLM for vector/matrix math
  - Dear ImGui for GUI

### Technical Features
  - Infix-to-prefix expression parsing
    - operator precedence, parenthesis, etc.
  - Stack-based prefix expression evaluation
  - Multithreaded graph generation
  - Basic lighting shaders
  - Pretty nice interface

### Screenshots
![graph](https://i.imgur.com/B7mOStw.png)
