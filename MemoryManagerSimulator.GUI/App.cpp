#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h> // brings in system openGL headers
#include <memory>

#include "Icon.h"
#include "MemoryManagerGuiContext.h"

// some OSes complain about OpenGL being bad and Metal being the best (cough cough Apple...)
#define GL_SILENCE_DEPRECATION

// window size
constexpr int WINDOW_SIZE_X = 624;
constexpr int WINDOW_SIZE_Y = 640;

using namespace std;

int WinMain()
{
	bool divideDpiScaling = false;
	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char *glsl_version = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.3 + GLSL 120
	const char *glsl_version = "#version 120";
	divideDpiScaling = true;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);		   // Required on Mac
#else
	// GL 3.3 + GLSL 130
	auto glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

	// setup window manager
	if (!glfwInit())
		return 1;

	// create window
	GLFWwindow* window = glfwCreateWindow(WINDOW_SIZE_X, WINDOW_SIZE_Y, "Memory Manager Simulator", nullptr, nullptr);

	// set the window size relative to display scaling
	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);
	// do not scale on macOS Retina
	if (divideDpiScaling)
	{
		xScale /= 2;
		yScale /= 2;
	}
	glfwSetWindowSize(window, WINDOW_SIZE_X * xScale, WINDOW_SIZE_Y * yScale);

	// exit program if we cannot create a window
	if (window == nullptr)
		return 1;

	// set application icon
	GLFWimage images[1];
	images[0].pixels = icon;
	images[0].height = 512;
	images[0].width = 512;
	glfwSetWindowIcon(window, 1, images);

	// set our current window to be the main one
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// on some OS, framebuffer and window size are not equal
	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);
	glViewport(0, 0, screen_width, screen_height);

	// set background color
	glClearColor(0.2f, 0.2f, 0.2f, 1);

	// create main ImGui context
	const auto appContext = make_shared<MemoryManagerGuiContext>(window, glsl_version, divideDpiScaling);

	// main render loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);
		appContext->NewFrame();
		appContext->Update();
		appContext->Render();
		glfwSwapBuffers(window);
	}
	// appContext.reset();	// when doing code analysis for memory leaks, set appContext to not be const and reset it to clear heap
	return 0;
}

// in case compiler is looking for main(), call WinMain() instead
int main()
{
	WinMain();
}
