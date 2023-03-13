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


	void ShowPhysicalMemoryControl() const;
	void ShowVirtualMemoryControl(int& currentJobIdx, int& currentJobId) const;
	void ShowConstantEditor();
	void ShowCreateJobControl() const;
	void ShowJobOperationControl(int& currentJobIdx, int& currentJobId, ReplacementMethod& method, int& addressToAccess, int& physicalAddress);
	void ShowJobSelector(int& currentJobIdx, int& currentJobId) const;
	void UpdateConstantValues(uint64_t &tmpPageSize, uint64_t& tmpPhysicalMemorySize, uint64_t& tmpVirtualMemorySize,
		uint64_t& tmpOffsetBits, uint64_t& tmpInstructionBits, uint64_t& tmpPageBits, uint64_t& tmpVirtualPages, uint64_t& tmpPhysicalPages) const;
};
