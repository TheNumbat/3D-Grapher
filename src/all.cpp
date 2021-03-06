
#include <codecvt>
#include <locale>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <thread>
#include <limits>
#include <mutex>

std::string trim_end(std::string s) {
	return s.substr(0, s.find_first_of('\0'));
}

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

#include <exprtk.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include <glew/glew.h>
#include <imgui/imgui.h>

#include "../asset/font.h"
#include "cam.h"
#include "settings.h"
#include "gl.h"
#include "graph.h"
#include "imgui_impl.h"
#include "state.h"

int main(int, char**) {

	state st;
	st.run();

	return 0;
}

#include "gl.cpp"
#include "graph.cpp"
#include "imgui_impl.cpp"
#include "state.cpp"
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>
