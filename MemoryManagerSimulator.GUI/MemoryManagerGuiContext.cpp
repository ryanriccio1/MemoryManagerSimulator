#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <string>
#include <cmath>
#include <chrono>
#include <random>

#include "MemoryManagerGuiContext.h"
#include "MemoryManagerWrapper.h"
#include "ConsolaTTF.h"

#pragma warning(disable : 4996)

MemoryManagerGuiContext::MemoryManagerGuiContext(GLFWwindow *window, const char *glsl_version, bool divideDpiScaling) : ImGuiDataContext(window, glsl_version), window(window)
{
	srand(NULL);
	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);
	dpiScaleFactor = xScale;
	if (divideDpiScaling)
		dpiScaleFactor /= 2;

	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(ConsolaTTF_compressed_data_base85, 13 * dpiScaleFactor);
	largeFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(ConsolaTTF_compressed_data_base85, 26 * dpiScaleFactor);
	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowRounding = 6.0f;
	ImGui::GetStyle().ScaleAllSizes(dpiScaleFactor);

	 windowFlags = 0;
	 windowFlags |= ImGuiWindowFlags_NoTitleBar;
	 windowFlags |= ImGuiWindowFlags_NoMove;
	 windowFlags |= ImGuiWindowFlags_NoResize;
	 windowFlags |= ImGuiWindowFlags_NoCollapse;
}

void MemoryManagerGuiContext::Update()
{

	// render your GUI
	static int addressToAccess;
	static int currentPhysicalAddress;
	static int currentJobId;
	static int currentJobIdx;
	static bool runningSimulation;
	static ReplacementMethod method;

	UpdateWindowSize();
	ImGui::Begin("Memory Manager Simulator", nullptr, windowFlags);              // Create a window called "Hello, world!" and append into it.
	if (ImGui::Button("Change Settings") || firstRun)
	{
		currentJobId = -1;
		runningSimulation = false;
		currentJobIdx = -1;
		method = LRU;
		ImGui::OpenPopup("Set Constants");
	}

	ShowConstantEditor();
	ShowCreateJobControl();
	ImGui::SameLine();
	ShowSimulation(runningSimulation, addressToAccess, currentPhysicalAddress, currentJobId, currentJobIdx, method);
	ShowJobOperationControl(currentJobIdx, currentJobId, method, addressToAccess, currentPhysicalAddress, runningSimulation);
	ImGui::SameLine();
	ShowInstructionInspectionControl(addressToAccess, currentPhysicalAddress);

	ShowVirtualMemoryControl(currentJobIdx, currentJobId);
	ImGui::SameLine();
	ShowPhysicalMemoryControl();
	
	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	//ImGui::ShowDemoWindow();
	ImGui::End();
}

void MemoryManagerGuiContext::ShowPhysicalMemoryControl() const
{
	ImGui::BeginChild(6, ImVec2(300 * dpiScaleFactor, 300 * dpiScaleFactor), true);
	ImGui::Text("Physical Memory");
	ImGui::Separator();

	ImGuiTableFlags tableFlags = 0;
	tableFlags |= ImGuiTableFlags_Borders;
	tableFlags |= ImGuiTableFlags_RowBg;

	if (ImGui::BeginTable("table1", 5, tableFlags))
	{
		// Display headers so we can inspect their interaction with borders.
		// (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for detail
		ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn("Addr", columnFlags, 45.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("IDX", columnFlags, 25.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Job", columnFlags, 0);
		ImGui::TableSetupColumn("VPg", columnFlags, 0);
		ImGui::TableSetupColumn("Virtual Address", columnFlags, 0);
		ImGui::TableHeadersRow();

		int physicalAddress = 0;
		if (memoryManager)
		{
			for (size_t row = 0; row < memoryManager->PHYSICAL_PAGES; row++)
			{
				ImGui::TableNextRow();
				for (size_t col = 0; col < 5; col++)
				{
					ImGui::TableSetColumnIndex(col);
					ImGui::AlignTextToFramePadding();
					if (memoryManager && memoryManager->memoryManager->jobManager->jobs->length > 0)
					{
						VirtualMemoryPage* virtualPage = memoryManager->memoryManager->physicalMemoryPages[row]->virtualMemoryPage;
						if (col == 0)
						{
							std::string addr = fmt::format("{:#x}", physicalAddress);
							ImGui::Text(addr.c_str());
							physicalAddress += memoryManager->PAGE_SIZE;
						}
						if (col == 1)
						{
							std::string index = std::to_string(memoryManager->memoryManager->physicalMemoryPages[row]->index);
							ImGui::Text(index.c_str());
						}
						if (col == 2)
						{
							if (virtualPage)
							{
								size_t jobId;
								for (size_t jobIdx = 0; jobIdx < memoryManager->memoryManager->jobManager->jobs->length; jobIdx++)
								{
									Job* job = (Job*)memoryManager->memoryManager->jobManager->jobs->getByIndex(memoryManager->memoryManager->jobManager->jobs, jobIdx);
									for (size_t pgIdx = 0; pgIdx < memoryManager->VIRTUAL_PAGES; pgIdx++)
									{
										if (job->virtualMemoryPages[pgIdx] == virtualPage)
											jobId = job->id;
									}
								}
								ImGui::Text(std::to_string(jobId).c_str());
							}
						}
						if (col == 3)
						{
							if (virtualPage)
							{
								ImGui::Text(std::to_string(virtualPage->index).c_str());
							}
						}
						if (col == 4)
						{
							if (virtualPage)
							{
								const int startingAddress = memoryManager->PAGE_SIZE * virtualPage->index;
								const int endingAddress = startingAddress + memoryManager->PAGE_SIZE - 1;
								auto addressRange = fmt::format("{:#x}->{:#x}", startingAddress, endingAddress);
								ImGui::Text(fmt::format("{:>15}", addressRange).c_str());
							}
						}
					}
					else
						ImGui::Text("");
				}
			}
		}
		ImGui::EndTable();
	}

	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowVirtualMemoryControl(int &currentJobIdx, int &currentJobId) const
{
	ImGui::BeginChild(5, ImVec2(300 * dpiScaleFactor, 300 * dpiScaleFactor), true);
	ImGui::Text("Virtual Memory");
	ImGui::Separator();

	ImGuiTableFlags tableFlags = 0;
	tableFlags |= ImGuiTableFlags_Borders;
	tableFlags |= ImGuiTableFlags_RowBg;

	if (ImGui::BeginTable("table1", 5, tableFlags))
	{
		// Display headers so we can inspect their interaction with borders.
		// (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for detail
		ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn("Addr", columnFlags, 45.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("IDX", columnFlags, 25.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Val", columnFlags, 25.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Ref", columnFlags, 25.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Physical Page", columnFlags, 0);
		ImGui::TableHeadersRow();

		Job *job = nullptr;
		if (memoryManager)
		{
			if (currentJobId >= 0)
				job = (Job*)memoryManager->memoryManager->jobManager->jobs->getByIndex(memoryManager->memoryManager->jobManager->jobs, currentJobIdx);
			int virtualAddress = 0;
			for (size_t row = 0; row < memoryManager->VIRTUAL_PAGES; row++)
			{
				ImGui::TableNextRow();
				for (size_t col = 0; col < 5; col++)
				{
					ImGui::TableSetColumnIndex(col);
					ImGui::AlignTextToFramePadding();
					if (job)
					{
						if (col == 0)
						{
							std::string addr = fmt::format("{:#x}", virtualAddress);
							ImGui::Text(addr.c_str());
							virtualAddress += memoryManager->PAGE_SIZE;
						}
						if (col == 1)
						{
							std::string index = std::to_string(job->virtualMemoryPages[row]->index);
							ImGui::Text(index.c_str());
						}
						if (col == 2)
						{
							std::string valid = std::to_string(job->virtualMemoryPages[row]->valid);
							ImGui::Text(valid.c_str());
						}
						if (col == 3)
						{
							std::string refCount = std::to_string(job->virtualMemoryPages[row]->refCount);
							ImGui::Text(refCount.c_str());
						}
						if (col == 4)
						{
							PhysicalMemoryPage* physicalPage = job->virtualMemoryPages[row]->physicalMemoryPage;
							if (physicalPage)
							{
								auto physicalPageInfo = fmt::format("{} - {:#x}", physicalPage->index, physicalPage->physicalAddress);
								ImGui::Text(fmt::format("{:>14}", physicalPageInfo).c_str());
							}
							else
								ImGui::Text("");
						}
					}
					else
						ImGui::Text("");
				}
			}
		}
		ImGui::EndTable();
	}

	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowConstantEditor()
{
	static uint64_t tmpPageSize;
	static uint64_t tmpPhysicalMemorySize;
	static uint64_t tmpVirtualMemorySize;

	static uint64_t tmpOffsetBits;
	static uint64_t tmpInstructionBits;
	static uint64_t tmpPageBits;
	static uint64_t tmpVirtualPages;
	static uint64_t tmpPhysicalPages;


	if (firstRun)
	{
		tmpPageSize = 16;
		tmpPhysicalMemorySize = 128;
		tmpVirtualMemorySize = 1024;
	}

	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Set Constants", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (memoryManager)
		{
			tmpPageSize = memoryManager->PAGE_SIZE;
			tmpPhysicalMemorySize = memoryManager->PHYSICAL_MEMORY_SIZE;
			tmpVirtualMemorySize = memoryManager->VIRTUAL_MEMORY_SIZE;

			tmpOffsetBits = memoryManager->OFFSET_BITS;
			tmpInstructionBits = memoryManager->INSTRUCTION_BITS;
			tmpPageBits = memoryManager->PAGE_BITS;
			tmpVirtualPages = memoryManager->VIRTUAL_PAGES;
			tmpPhysicalPages = memoryManager->PHYSICAL_PAGES;
			memoryManager.reset();
		}
		ImGui::Text("Constants:");
		ImGui::Separator();

		ImGui::InputInt("Page Size", (int*)&tmpPageSize, 16, 128);
		if (tmpPageSize < 1)
		{
			tmpPageSize = 16;
		}
		if (tmpPageSize % 16 != 0)
		{
			tmpPageSize -= tmpPageSize % 16;
		}
		if (tmpPhysicalMemorySize < tmpPageSize ||
			tmpVirtualMemorySize < tmpPageSize)
		{
			tmpPageSize = std::min(tmpPhysicalMemorySize, tmpVirtualMemorySize);
		}

		ImGui::InputInt("Physical Memory Size", (int*)&tmpPhysicalMemorySize, 16, 128);
		if (tmpPhysicalMemorySize < 1)
		{
			tmpPhysicalMemorySize = 16;
		}
		if (tmpPhysicalMemorySize % 16 != 0)
		{
			tmpPhysicalMemorySize -= tmpPhysicalMemorySize % 16;
		}

		ImGui::InputInt("Virtual Memory Size", (int*)&tmpVirtualMemorySize, 16, 128);
		if (tmpVirtualMemorySize < 1)
		{
			tmpVirtualMemorySize = 16;
		}
		if (tmpVirtualMemorySize % 16 != 0)
		{
			tmpVirtualMemorySize -= tmpVirtualMemorySize % 16;
		}

		ImGui::Text("Virtual Pages:\t    %lld", tmpVirtualPages);
		ImGui::Text("Physical Pages:\t   %lld", tmpPhysicalPages);
		ImGui::Text("Offset Bits:\t      %lld", tmpOffsetBits);
		ImGui::Text("Page Bits:\t        %lld", tmpPageBits);
		ImGui::Text("Instruction Bits:\t %lld", tmpInstructionBits);

		UpdateConstantValues(tmpPageSize, tmpPhysicalMemorySize, tmpVirtualMemorySize,
			tmpOffsetBits, tmpInstructionBits, tmpPageBits, tmpVirtualPages, tmpPhysicalPages);

		firstRun = false;


		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			memoryManager = std::make_shared<MemoryManagerSimulatorWrapper>(tmpPageSize, tmpPhysicalMemorySize, tmpVirtualMemorySize);
		}
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}


void MemoryManagerGuiContext::ShowCreateJobControl() const
{
	ImGui::BeginChild(3, ImVec2(300 * dpiScaleFactor, 118 * dpiScaleFactor), true);
	ImGui::Text("Create Job");
	ImGui::Separator();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Job Id:");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);
	static int jobId;
	ImGui::InputInt("##label_jobId", &jobId, 0, 0);

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Job Name:");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);
	static char jobName[128];
	ImGui::InputText("##label_jobName", jobName, 128);

	ImGui::Spacing();

	if (ImGui::Button("Create Job", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
	{
		char *tempJobName = new char[128];
		strcpy(tempJobName, jobName);
		memoryManager->m_createJob(tempJobName, jobId);
		jobId = 0;
		strcpy(jobName, "");
	}

	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowJobOperationControl(int &currentJobIdx, int &currentJobId, ReplacementMethod &method, int &addressToAccess, int& physicalAddress, bool& runningSimulation)
{
	ImGui::BeginChild(4, ImVec2(300 * dpiScaleFactor, 175 * dpiScaleFactor), true);
	ImGui::Text("Job Operations");
	ImGui::Separator();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Job:");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);

	if (memoryManager)
		ImGui::BeginDisabled(memoryManager->memoryManager->jobManager->jobs->length < 1 || runningSimulation);
	ShowJobSelector(currentJobIdx, currentJobId);
	if (memoryManager)
		ImGui::EndDisabled();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Free:");

	static const char *comboPreview;
	if (method >= 0 && method < 3)
		comboPreview = replacementMethodString[method];
	else
		comboPreview = "";

	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);
	if (memoryManager)
		ImGui::BeginDisabled(memoryManager->memoryManager->jobManager->jobs->length < 1 || runningSimulation);
	if (ImGui::BeginCombo("##label_replacementMethod", comboPreview))
	{
		for (size_t idx = 0; idx < IM_ARRAYSIZE(replacementMethodString); idx++)
		{
			const bool is_selected = (method == idx);

			if (ImGui::Selectable(replacementMethodString[idx], is_selected))
				method = (ReplacementMethod)idx;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (memoryManager)
		ImGui::EndDisabled();

	ImGuiInputTextFlags inputFlags = 0;
	inputFlags |= ImGuiInputTextFlags_CharsHexadecimal;
	// inputFlags |= ImGuiInputTextFlags_EnterReturnsTrue;
	if (currentJobIdx == -1)
		inputFlags |= ImGuiInputTextFlags_ReadOnly;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Address:");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);

	ImGui::BeginDisabled(currentJobIdx == -1 || runningSimulation);
	ImGui::InputInt("##label_addressToAccess", &addressToAccess, 0, 0, inputFlags);
	if (memoryManager)
	{
		if (addressToAccess < 0 || addressToAccess > memoryManager->VIRTUAL_MEMORY_SIZE)
			addressToAccess = 0;
	}

	ImGui::Spacing();
	if (ImGui::Button("Access Job", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
	{
		physicalAddress = (uint64_t)memoryManager->m_accessJob(currentJobId, addressToAccess, method);
	}
	if (ImGui::Button("Remove Job", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
	{
		memoryManager->m_removeJob(currentJobId);
		currentJobId = -1;
		currentJobIdx = -1;
	}
	ImGui::EndDisabled();
	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowJobSelector(int &currentJobIdx, int &currentJobId) const
{
	std::string combo_preview_value;
	LinkedList *jobList = nullptr;
	if (memoryManager)
	{
		jobList = memoryManager->memoryManager->jobManager->jobs;
		if (jobList->length > 0)
		{
			const Job *selectedJob = static_cast<Job *>(jobList->getByIndex(jobList, currentJobIdx)); // Pass in the preview value visible before opening the combo (it could be anything)
			if (selectedJob)
				combo_preview_value = std::to_string(selectedJob->id);
		}
	}

	if (ImGui::BeginCombo("##label_selectedJob", combo_preview_value.c_str()))
	{
		if (memoryManager && jobList)
		{
			for (size_t idx = 0; idx < jobList->length; idx++)
			{
				Job *currentJob = static_cast<Job *>(jobList->getByIndex(jobList, idx));

				const bool is_selected = (currentJobIdx == idx);
				if (currentJob)
				{
					std::string jobIdString = std::to_string(currentJob->id);
					std::string jobNameString = currentJob->name;
					std::string label = fmt::format("{:<5} | {:>10}", jobIdString, jobNameString);
					if (ImGui::Selectable(label.c_str(), is_selected))
					{
						currentJobId = currentJob->id;
						currentJobIdx = static_cast<int>(idx);
					}
					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
					{
						currentJobId = static_cast<int>(currentJob->id);
						ImGui::SetItemDefaultFocus();
					}
				}
			}
		}
		ImGui::EndCombo();
	}
}

void MemoryManagerGuiContext::ShowInstructionInspectionControl(int &addressToAccess, int &currentPhysicalAddress)
{
	ImGui::BeginChild(7, ImVec2(300 * dpiScaleFactor, 175 * dpiScaleFactor), true);
	ImGui::Text("Instruction Inspection");
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::PushFont(largeFont);

	if (memoryManager)
	{

		for (size_t idx = 0; idx < memoryManager->PAGE_BITS; idx++)
		{
			unsigned int tmp = (unsigned int)addressToAccess << (sizeof(unsigned int) * 8 - memoryManager->INSTRUCTION_BITS + idx) >> ((sizeof(unsigned int) * 8) - 1);
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), std::to_string(tmp).c_str());
		}

		for (size_t idx = 0; idx < memoryManager->OFFSET_BITS; idx++)
		{
			unsigned int tmp = (unsigned int)addressToAccess << (sizeof(unsigned int) * 8 - memoryManager->OFFSET_BITS + idx) >> ((sizeof(unsigned int) * 8) - 1);
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), std::to_string(tmp).c_str());
		}

		ImGui::PopFont();

		ImGui::Text("Page Index:    ");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), std::to_string(addressToAccess >> memoryManager->OFFSET_BITS).c_str());

		ImGui::Text("Page Offset:   ");
		ImGui::SameLine();
		unsigned int offset = (unsigned int)addressToAccess << (sizeof(addressToAccess) * 8 - memoryManager->OFFSET_BITS) >> (sizeof(addressToAccess) * 8 - memoryManager->OFFSET_BITS);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), std::to_string(offset).c_str());
	}
	else
	{
		for (size_t idx = 0; idx < 10; idx++)
		{
			ImGui::SameLine();
			ImGui::Text("0");
		}
		ImGui::PopFont();
		ImGui::Text("Page Index:    ");
		ImGui::Text("Page Offset:   ");
	}
	for (size_t idx = 0; idx < 6; idx++)
		ImGui::Spacing();
		
	ImGui::Separator();

	ImGui::Text("Last Physical Address:");

	ImGui::PushFont(largeFont);
	ImGui::Text(fmt::format("{:#08x}", currentPhysicalAddress).c_str());
	ImGui::PopFont();

	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowSimulation(bool &runningSimulation, int &addressToAccess, int &currentPhysicalAddress, int &currentJobId, int &currentJobIdx, ReplacementMethod &method)
{
	static std::chrono::duration<long long, std::ratio<1, 1000>> endTime;
	static std::chrono::duration<long long, std::ratio<1, 1000>> lastTime;
	static int currentIteration;

	ImGui::BeginChild(8, ImVec2(300 * dpiScaleFactor, 118 * dpiScaleFactor), true);
	ImGui::Text("Simulate");
	ImGui::Separator();

	ImGui::BeginDisabled(runningSimulation);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Iterations:");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);
	static int simIterations;
	ImGui::InputInt("##label_simIterations", &simIterations, 0, 0);
	if (simIterations < 1)
		simIterations = 1;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Time (ms):");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);
	static int simTime;
	ImGui::InputInt("##label_simTime", &simTime, 0, 0);
	if (simTime < 10)
		simTime = 10;
	ImGui::EndDisabled();

	ImGui::Spacing();

	const auto currentTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	if (runningSimulation)
	{
		if (ImGui::Button("Stop Simulation", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
		{
			runningSimulation = false;
			currentIteration = 0;
		}

		if (memoryManager)
		{
			if (currentTime > lastTime)
			{
				if (currentIteration < simIterations)
				{
					addressToAccess = rand() % memoryManager->VIRTUAL_MEMORY_SIZE;
					currentPhysicalAddress = (uint64_t)memoryManager->m_accessJob(currentJobId, addressToAccess, method);
					lastTime += std::chrono::milliseconds(simTime);
					currentIteration++;
				}
				else
				{
					currentIteration = 0;
					runningSimulation = false;
				}
			}
		}
	}
	if (!runningSimulation)
	{
		ImGui::BeginDisabled(currentJobIdx == -1 || currentTime < endTime);
		if (ImGui::Button("Run Simulation", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
		{
			runningSimulation = true;
			currentIteration = 0;
			endTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + std::chrono::milliseconds(simTime * simIterations);
			lastTime = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) + std::chrono::milliseconds(simTime);
		}
		ImGui::EndDisabled();
	}
	ImGui::EndChild();
}


void MemoryManagerGuiContext::UpdateWindowSize()
{
	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);

	windowSize = ImVec2(static_cast<float>(screen_width), static_cast<float>(screen_height));

	ImGui::SetNextWindowSize(windowSize);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
}


void MemoryManagerGuiContext::UpdateConstantValues(uint64_t& tmpPageSize, uint64_t& tmpPhysicalMemorySize, uint64_t& tmpVirtualMemorySize,
	uint64_t& tmpOffsetBits, uint64_t& tmpInstructionBits, uint64_t& tmpPageBits, uint64_t& tmpVirtualPages, uint64_t& tmpPhysicalPages) const
{
	tmpOffsetBits = MemoryManagerSimulatorWrapper::m_uint64log2(tmpPageSize);
	if (tmpVirtualMemorySize != 0)
		tmpInstructionBits = MemoryManagerSimulatorWrapper::m_uint64log2(tmpVirtualMemorySize);
	tmpPageBits = tmpInstructionBits - tmpOffsetBits;
	if (tmpPageSize != 0)
	{
		tmpVirtualPages = tmpVirtualMemorySize / tmpPageSize;
		tmpPhysicalPages = tmpPhysicalMemorySize / tmpPageSize;
	}
	else
	{
		tmpVirtualPages = 0;
		tmpPhysicalPages = 0;
	}
}
