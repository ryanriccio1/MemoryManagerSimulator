#pragma once

#include <memory>

#include "MemoryManagerWrapper.h"
#include "ImGuiDataContext.h"

class MemoryManagerGuiContext : public ImGuiDataContext
{
public:
	static inline bool firstRun = true;
	static inline const char* replacementMethodString[] = { "LRU", "LFU", "FIFO" };

	MemoryManagerGuiContext(GLFWwindow* window, const char* glsl_version);
	void Update() override;
	void UpdateWindowSize();
private:
	std::shared_ptr<MemoryManagerSimulatorWrapper> memoryManager;
	ImVec2 windowSize;
	GLFWwindow* window;
	float dpiScaleFactor;
	ImGuiWindowFlags windowFlags;
	ImFont* largeFont;

	uint64_t uint64log2(uint64_t input) const;


	void ShowPhysicalMemoryControl() const;
	void ShowVirtualMemoryControl(int& currentJobIdx, int& currentJobId) const;
	void ShowConstantEditor();
	void ShowCreateJobControl() const;
	void ShowJobOperationControl(int& currentJobIdx, int& currentJobId, ReplacementMethod& method, int& addressToAccess, int& physicalAddress);
	void ShowJobSelector(int& currentJobIdx, int& currentJobId) const;
	void UpdateConstantValues() const;
};
