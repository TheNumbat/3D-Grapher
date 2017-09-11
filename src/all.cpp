
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

#include <glm/include/glm.hpp>
#include <glm/include/gtc/matrix_transform.hpp>
#include <glm/include/gtc/type_ptr.hpp>

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>

#include <imgui/imgui.h>

using namespace glm;
using namespace std;

#include "cam.h"
#include "exp.h"
#include "gl.h"
#include "graph.h"
#include "imgui_impl.h"
#include "settings.h"
#include "state.h"

int main(int, char**) {

	state st;
	st.run();

	return 0;
}

#include "exp.cpp"
#include "gl.cpp"
#include "graph.cpp"
#include "imgui_impl.cpp"
#include "state.cpp"

#include "..\deps\imgui\imgui.cpp" 
#include "..\deps\imgui\imgui_draw.cpp" 
