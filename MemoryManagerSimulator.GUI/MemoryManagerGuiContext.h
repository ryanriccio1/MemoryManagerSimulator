#pragma once
#include <memory>

#include "MemoryManagerWrapper.h"
#include "ImGuiDataContext.h"

/**
 * 	MemoryManager implementation of ImGui GLFW Window.
 */
class MemoryManagerGuiContext : public ImGuiDataContext
{
public:
	static inline bool firstRun = true;
	static inline const char *replacementMethodString[] = {"LRU", "LFU", "FIFO"};

	/**
	 * @brief Construct a new Memory Manager Gui Context object.
	 *
	 * @param window Window to render to.
	 * @param glsl_version GL shader version.
	 * @param divideDpiScaling If on macOS Retina Display, scaling factor is 2.0, yet OS handles scaling so divide scaling by 2.
	 */
	MemoryManagerGuiContext(GLFWwindow *window, const char *glsl_version, bool divideDpiScaling);
	/**
	 * Main frame defination function.
	 */
	void Update() override;

private:
	std::shared_ptr<MemoryManagerSimulatorWrapper> memoryManager;
	ImVec2 windowSize;			  // current window size
	GLFWwindow *window;			  // window to render to
	float dpiScaleFactor;		  // Global scaling factor used for HiDPI.
	ImGuiWindowFlags windowFlags; // Display flags for scaling, title bar, resize, etc.
	ImFont *largeFont;			  // Non-default large font (2x size of default font)

	/**
	 * Update ImGui window to size of GLFW window.
	 */
	void UpdateWindowSize();
	/**
	 * @brief Using references to existing variables, update them to the current calculated value based on the input values.
	 *
	 * @param tmpPageSize (Input) Size of individual pages.
	 * @param tmpPhysicalMemorySize (Input) Entire size of physical memory shared for entire program.
	 * @param tmpVirtualMemorySize (Input) Amount of virtual memory each job has access to.
	 * @param tmpOffsetBits (Output) Bits needed to specify address offset from within a page.
	 * @param tmpPageBits (Output) Bits needed to specify the virtual page index.
	 * @param tmpInstructionBits (Output) Bits needed to specify entire location within virtual memory.
	 * @param tmpPhysicalPages (Output) Number of physical pages for entire memory manager.
	 * @param tmpVirtualPages (Output) Number of virtual pages per job.
	 */
	void UpdateConstantValues(uint64_t &tmpPageSize, uint64_t &tmpPhysicalMemorySize, uint64_t &tmpVirtualMemorySize,
							  uint64_t &tmpOffsetBits, uint64_t &tmpPageBits, uint64_t &tmpInstructionBits, uint64_t &tmpPhysicalPages, uint64_t &tmpVirtualPages) const;

	// GUI controls as individual functions (all GUIs are ImGui children).
	void ShowConstantEditor();
	void ShowCreateJobControl() const;
	void ShowSimulation(bool &runningSimulation, int &addressToAccess, int &currentPhysicalAddress, int &currentJobId, int &currentJobIdx, ReplacementMethod &method);
	void ShowJobSelector(int &currentJobIdx, int &currentJobId) const;
	void ShowJobOperationControl(int &currentJobIdx, int &currentJobId, ReplacementMethod &method, int &addressToAccess, int &physicalAddress, bool &runningSimulation);
	void ShowInstructionInspectionControl(int &addressToAccess, int &currentPhysicalAddress);
	void ShowVirtualMemoryControl(int &currentJobIdx, int &currentJobId) const;
	void ShowPhysicalMemoryControl() const;
};
