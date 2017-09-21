
#include <codecvt>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <stack>
#include <queue>
#include <algorithm>
#include <exception>
#include <cmath>
#include <thread>
#include <limits>

#include <glm/include/glm.hpp>
#include <glm/include/gtc/matrix_transform.hpp>
#include <glm/include/gtc/type_ptr.hpp>

#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_syswm.h>

#include <imgui/imgui.h>

using namespace glm;
using namespace std;

#include "gl.h"
#include "cam.h"
#include "exp.h"
#include "graph.h"
#include "imgui_impl.h"
#include "settings.h"
#include "state.h"

std::wstring utf8_to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

std::string wstring_to_utf8(const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

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
