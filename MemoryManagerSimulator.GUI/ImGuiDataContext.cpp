#include "ImGuiDataContext.h"

ImGuiDataContext::ImGuiDataContext(GLFWwindow *window, const char *glsl_version)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// platform/renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

ImGuiDataContext::~ImGuiDataContext()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiDataContext::NewFrame()
{
	// pass inputs from platform/renderer to imgui and start frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiDataContext::Update()
{
	// default update (should be overridden)
	ImGui::Begin("ImGui Window");
	ImGui::End();
}

void ImGuiDataContext::Render()
{
	// Render imgui context to screen
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
