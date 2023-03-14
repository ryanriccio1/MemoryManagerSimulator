#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

/**
 * Base data class for ImGui rendering to GLFW/OpenGL3.0 backend.
 */
class ImGuiDataContext
{
public:
	/**
	 * @brief Construct a new Im Gui Data Context object.
	 *
	 * @param window GLFW window to render to.
	 * @param glsl_version GL shader version.
	 */
	ImGuiDataContext(GLFWwindow *window, const char *glsl_version);
	virtual ~ImGuiDataContext();

	/**
	 * Render a new frame using GLFW and OpenGL3.
	 */
	void NewFrame();
	/**
	 * Update the new frame.
	 */
	virtual void Update();
	/**
	 * Render new frame.
	 */
	void Render();
};