#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// needed for glfl <= 3.0
//#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
//#pragma comment(lib, "legacy_stdio_definitions")
//#endif

class ImGuiDataContext
{
public:
	ImGuiDataContext(GLFWwindow* window, const char* glsl_version);
	virtual ~ImGuiDataContext();
	void NewFrame();
	virtual void Update();
	void Render();
};