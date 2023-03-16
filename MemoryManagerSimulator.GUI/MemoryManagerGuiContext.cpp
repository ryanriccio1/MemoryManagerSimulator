#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <string>
#include <cmath>
#include <chrono>
#include <random>

#include "MemoryManagerGuiContext.h"
#include "MemoryManagerWrapper.h"
#include "ConsolaTTF.h"

// use deprecated string functions
#pragma warning(disable : 4996)

using namespace std;

MemoryManagerGuiContext::MemoryManagerGuiContext(GLFWwindow* window, const char* glsl_version,
                                                 bool divideDpiScaling) : ImGuiDataContext(window, glsl_version),
                                                                          window(window)
{
	// set random seed
	srand(chrono::system_clock::now().time_since_epoch().count());

	// set scaling based on display scaling
	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);
	dpiScaleFactor = xScale;
	// do not scale on macos
	if (divideDpiScaling)
		dpiScaleFactor /= 2;
	// scale all ImGui controls (except fonts)
	ImGui::GetStyle().ScaleAllSizes(dpiScaleFactor);

	// set fonts from compressed file (ConsolaTTF.h)
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(ConsolaTTF_compressed_data_base85, 13 * dpiScaleFactor);
	largeFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(ConsolaTTF_compressed_data_base85, 26 * dpiScaleFactor);
	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowRounding = 6.0f;

	// set ImGui window options
	windowFlags = 0;
	windowFlags |= ImGuiWindowFlags_NoTitleBar;
	windowFlags |= ImGuiWindowFlags_NoMove;
	windowFlags |= ImGuiWindowFlags_NoResize;
	windowFlags |= ImGuiWindowFlags_NoCollapse;
}

void MemoryManagerGuiContext::Update()
{
	static int addressToAccess;
	static int currentPhysicalAddress;
	static int currentJobId;
	static int currentJobIdx;
	static bool runningSimulation;
	static ReplacementMethod method;

	// make ImGui window size of GLFW window
	UpdateWindowSize();

	// create main window
	ImGui::Begin("Memory Manager Simulator", nullptr, windowFlags);

	// config setup for first run
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
	ShowJobOperationControl(currentJobIdx, currentJobId, method, addressToAccess, currentPhysicalAddress,
	                        runningSimulation);
	ImGui::SameLine();
	ShowInstructionInspectionControl(addressToAccess, currentPhysicalAddress);

	ShowVirtualMemoryControl(currentJobIdx, currentJobId);
	ImGui::SameLine();
	ShowPhysicalMemoryControl();

	// see current FPS
	// ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	// ImGui::ShowDemoWindow();
	ImGui::End();
}

void MemoryManagerGuiContext::UpdateWindowSize()
{
	// get the size of the frame buffer and scale our ImGui window accordingly
	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);

	windowSize = ImVec2(static_cast<float>(screen_width), static_cast<float>(screen_height));

	ImGui::SetNextWindowSize(windowSize);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
}

void MemoryManagerGuiContext::UpdateConstantValues(const uint64_t& tmpPageSize, const uint64_t& tmpPhysicalMemorySize,
                                                   const uint64_t& tmpVirtualMemorySize,
                                                   uint64_t& tmpOffsetBits, uint64_t& tmpPageBits,
                                                   uint64_t& tmpInstructionBits, uint64_t& tmpPhysicalPages,
                                                   uint64_t& tmpVirtualPages)
{
	// calculate the values being used in the memory manager based on inputs so they are always up to date

	// number of bits needed to represent offset within page
	tmpOffsetBits = MemoryManagerSimulatorWrapper::m_uint64log2(tmpPageSize);

	// number of bits required to address all of virtual memory
	if (tmpVirtualMemorySize != 0)
		tmpInstructionBits = MemoryManagerSimulatorWrapper::m_uint64log2(tmpVirtualMemorySize);

	// number of bits required to get an index into virtual memory
	tmpPageBits = tmpInstructionBits - tmpOffsetBits;

	// set page sizes
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

void MemoryManagerGuiContext::ShowConstantEditor()
{
	static uint64_t tmpPageSize;
	static uint64_t tmpPhysicalMemorySize;
	static uint64_t tmpVirtualMemorySize;

	static uint64_t tmpOffsetBits;
	static uint64_t tmpPageBits;
	static uint64_t tmpInstructionBits;
	static uint64_t tmpPhysicalPages;
	static uint64_t tmpVirtualPages;

	// set default values on first run
	if (firstRun)
	{
		tmpPageSize = 16;
		tmpPhysicalMemorySize = 128;
		tmpVirtualMemorySize = 1024;
	}

	// always center this window when appearing
	const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	// begin popup
	if (ImGui::BeginPopupModal("Set Constants", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// if there's a memory manager that exists, get its configuration
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
			memoryManager.reset(); // release memory manager to prepare for new
		}
		ImGui::Text("Constants:");
		ImGui::Separator();

		// all inputs must be positive intervals of 16
		ImGui::InputInt("Page Size", (int*)(&tmpPageSize), 16, 128);
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
			// page size can never be larger than either PMem or VMem
			tmpPageSize = min(tmpPhysicalMemorySize, tmpVirtualMemorySize);
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

		// make constant values current
		UpdateConstantValues(tmpPageSize, tmpPhysicalMemorySize, tmpVirtualMemorySize,
		                     tmpOffsetBits, tmpPageBits, tmpInstructionBits, tmpPhysicalPages, tmpVirtualPages);

		// display calculated value
		ImGui::Text("Virtual Pages:\t    %lld", tmpVirtualPages);
		ImGui::Text("Physical Pages:\t   %lld", tmpPhysicalPages);
		ImGui::Text("Offset Bits:\t      %lld", tmpOffsetBits);
		ImGui::Text("Page Bits:\t        %lld", tmpPageBits);
		ImGui::Text("Instruction Bits:\t %lld", tmpInstructionBits);

		// make sure we know this is not our first run once we've executed this once
		firstRun = false;

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			ImGui::CloseCurrentPopup();
			// make a new memory manager
			memoryManager = make_shared<MemoryManagerSimulatorWrapper>(
				tmpPageSize, tmpPhysicalMemorySize, tmpVirtualMemorySize);
		}
		ImGui::SetItemDefaultFocus(); // focus on object in previous window
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
		// create a character buffer to copy our name char array into
		// jobName only lasts during this function since on stack
		// heap is freed on job removal
		auto tempJobName = new char[128];
		strcpy(tempJobName, jobName);
		memoryManager->m_createJob(tempJobName, jobId);

		// reset the static vars for this function
		jobId = 0;
		strcpy(jobName, "");
	}

	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowSimulation(bool& runningSimulation, int& addressToAccess, int& currentPhysicalAddress,
                                             const int& currentJobId, const int& currentJobIdx,
                                             const ReplacementMethod& method) const
{
	// store the times that things occur at
	static chrono::duration<long long, ratio<1, 1000>> endTime;
	static chrono::duration<long long, ratio<1, 1000>> lastTime;

	static int currentIteration;

	ImGui::BeginChild(8, ImVec2(300 * dpiScaleFactor, 118 * dpiScaleFactor), true);
	ImGui::Text("Simulate");
	ImGui::Separator();

	ImGui::BeginDisabled(runningSimulation); // disable if running a simulation
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Iterations:");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);
	static int simIterations;
	ImGui::InputInt("##label_simIterations", &simIterations, 0, 0);
	// we must have more than 1 iteration
	if (simIterations < 1)
		simIterations = 1;

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Time (ms):");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);
	static int simTime;
	ImGui::InputInt("##label_simTime", &simTime, 0, 0);
	// there must be more than 10ms between each calculation (arbitrary limit)
	if (simTime < 10)
		simTime = 10;
	ImGui::EndDisabled();

	ImGui::Spacing();

	// get current time
	const auto currentTime = duration_cast<chrono::milliseconds>(
		chrono::system_clock::now().time_since_epoch());

	if (runningSimulation)
	{
		// stop sim
		if (ImGui::Button("Stop Simulation", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
		{
			runningSimulation = false;
			endTime = chrono::milliseconds(0);
			currentIteration = 0;
		}

		// perform simulation
		if (memoryManager)
		{
			// if enough time has elapsed since last calculation
			if (currentTime > lastTime)
			{
				// if we haven't reached the end of our iterations
				if (currentIteration < simIterations)
				{
					// random value between 0 and (virtual memory size - 1)
					addressToAccess = rand() % memoryManager->VIRTUAL_MEMORY_SIZE;

					// perform access and calculate completion time
					currentPhysicalAddress = (uint64_t)memoryManager->
						m_accessJob(currentJobId, addressToAccess, method);
					lastTime += chrono::milliseconds(simTime);
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
	// do not use else so that if current frame stops sim, button state changes
	if (!runningSimulation)
	{
		ImGui::BeginDisabled(currentJobIdx == -1 || currentTime < endTime);
		if (ImGui::Button("Run Simulation", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
		{
			runningSimulation = true;
			currentIteration = 0;

			// calculate total time needed and setup lastTime for first iteration
			endTime = duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()) +
				chrono::milliseconds(simTime * simIterations);
			lastTime = duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()) +
				chrono::milliseconds(simTime);
		}
		ImGui::EndDisabled();
	}
	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowJobSelector(int& currentJobIdx, int& currentJobId) const
{
	string combo_preview_value;
	LinkedList* jobList = nullptr;
	if (memoryManager)
	{
		jobList = memoryManager->memoryManager->jobManager->jobs;
		if (jobList->length > 0)
		{
			// if we have any jobs, get the selected job
			const Job* selectedJob = static_cast<Job*>(jobList->getByIndex(jobList, currentJobIdx));
			// Pass in the preview value visible before opening the combo (it could be anything)
			// if the selected job exists, display it for the combo box
			if (selectedJob)
				combo_preview_value = to_string(selectedJob->id);
		}
	}

	if (ImGui::BeginCombo("##label_selectedJob", combo_preview_value.c_str()))
	{
		// if we have data to display in combo
		if (memoryManager && jobList)
		{
			// for each job (cannot use range based for since jobList is not iterable)
			for (size_t idx = 0; idx < jobList->length; idx++)
			{
				// get the job at an index
				const auto currentJob = static_cast<Job*>(jobList->getByIndex(jobList, idx));

				const bool is_selected = (currentJobIdx == idx);
				if (currentJob)
				{
					// create the label
					string jobIdString = to_string(currentJob->id);
					string jobNameString = currentJob->name;
					string label = fmt::format("{:<5} | {:>10}", jobIdString, jobNameString);

					// display the item
					if (ImGui::Selectable(label.c_str(), is_selected))
					{
						// if its selection status changes, update these values
						currentJobId = currentJob->id;
						currentJobIdx = static_cast<int>(idx);
					}
					if (is_selected)
					{
						// double check to make sure JobID is always current
						currentJobId = static_cast<int>(currentJob->id);
						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						ImGui::SetItemDefaultFocus();
					}
				}
			}
		}
		ImGui::EndCombo();
	}
}

void MemoryManagerGuiContext::ShowJobOperationControl(int& currentJobIdx, int& currentJobId, ReplacementMethod& method,
                                                      int& addressToAccess, int& physicalAddress,
                                                      const bool& runningSimulation) const
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

	// combo box for replacement method (LRU, LFU, FIFO)
	static const char* comboPreview;
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
				method = static_cast<ReplacementMethod>(idx);

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (memoryManager)
		ImGui::EndDisabled();

	// input fpor address
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
		// make sure address is less than virtual memory size
		if (addressToAccess < 0 || addressToAccess > memoryManager->VIRTUAL_MEMORY_SIZE)
			addressToAccess = 0;
	}

	ImGui::Spacing();
	if (ImGui::Button("Access Job", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
	{
		// get the physical address from the memory manager
		physicalAddress = (uint64_t)memoryManager->m_accessJob(currentJobId, addressToAccess, method);
	}
	if (ImGui::Button("Remove Job", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
	{
		// reset all job parameters and remove job from memory manager
		memoryManager->m_removeJob(currentJobId);
		currentJobId = -1;
		currentJobIdx = -1;
	}
	ImGui::EndDisabled();
	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowInstructionInspectionControl(const int& addressToAccess,
                                                               int& currentPhysicalAddress) const
{
	ImGui::BeginChild(7, ImVec2(300 * dpiScaleFactor, 175 * dpiScaleFactor), true);
	ImGui::Text("Instruction Inspection");
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::PushFont(largeFont);

	if (memoryManager)
	{
		// rather than creating static masks, calculate each bit at a specific position for each bit in the instruction

		// perform this for virtual page index
		for (size_t idx = 0; idx < memoryManager->PAGE_BITS; idx++)
		{
			// get the bit at a given index and shift back into the 0 position
			unsigned int tmp = static_cast<unsigned int>(addressToAccess) << (sizeof(unsigned int) * 8 - memoryManager->
				INSTRUCTION_BITS + idx) >> ((sizeof(unsigned int) * 8) - 1);
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), to_string(tmp).c_str());
		}

		// perform for offset within page
		for (size_t idx = 0; idx < memoryManager->OFFSET_BITS; idx++)
		{
			// get the bit at a given index and shift back into the 0 position
			unsigned int tmp = static_cast<unsigned int>(addressToAccess) << (sizeof(unsigned int) * 8 - memoryManager->
				OFFSET_BITS + idx) >> ((sizeof(unsigned int) * 8) - 1);
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), to_string(tmp).c_str());
		}

		ImGui::PopFont();

		// display calculations
		ImGui::Text("Page Index:    ");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f),
		                   to_string(addressToAccess >> memoryManager->OFFSET_BITS).c_str());

		ImGui::Text("Page Offset:   ");
		ImGui::SameLine();
		unsigned int offset = static_cast<unsigned int>(addressToAccess) << (sizeof(addressToAccess) * 8 - memoryManager
			->OFFSET_BITS) >> (sizeof(addressToAccess) * 8 - memoryManager->OFFSET_BITS);
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), to_string(offset).c_str());
	}
	else
	{
		// just display uncolored zeros if no memory manager
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

void MemoryManagerGuiContext::ShowVirtualMemoryControl(const int& currentJobIdx, const int& currentJobId) const
{
	ImGui::BeginChild(5, ImVec2(300 * dpiScaleFactor, 300 * dpiScaleFactor), true);
	ImGui::Text("Virtual Memory");
	ImGui::Separator();

	ImGuiTableFlags tableFlags = 0;
	tableFlags |= ImGuiTableFlags_Borders;
	tableFlags |= ImGuiTableFlags_RowBg;

	if (ImGui::BeginTable("table1", 5, tableFlags))
	{
		// setup table columns/headers
		const ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn("Addr", columnFlags, 45.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("IDX", columnFlags, 25.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Val", columnFlags, 25.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Ref", columnFlags, 25.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Physical Page", columnFlags, 0);
		ImGui::TableHeadersRow();

		Job* job = nullptr;
		if (memoryManager)
		{
			// get the current job
			if (currentJobId >= 0)
				job = static_cast<Job*>(memoryManager->memoryManager->jobManager->jobs->getByIndex(
					memoryManager->memoryManager->jobManager->jobs, currentJobIdx));
			int virtualAddress = 0;

			// for each row
			for (size_t row = 0; row < memoryManager->VIRTUAL_PAGES; row++)
			{
				ImGui::TableNextRow();
				// for each column
				for (size_t col = 0; col < 5; col++)
				{
					ImGui::TableSetColumnIndex(col);
					ImGui::AlignTextToFramePadding();
					if (job)
					{
						// format all data based on which column in the row we are in
						if (col == 0)
						{
							string addr = fmt::format("{:#x}", virtualAddress);
							ImGui::Text(addr.c_str());
							virtualAddress += memoryManager->PAGE_SIZE;
						}
						if (col == 1)
						{
							string index = to_string(job->virtualMemoryPages[row]->index);
							ImGui::Text(index.c_str());
						}
						if (col == 2)
						{
							string valid = to_string(job->virtualMemoryPages[row]->valid);
							ImGui::Text(valid.c_str());
						}
						if (col == 3)
						{
							string refCount = to_string(job->virtualMemoryPages[row]->refCount);
							ImGui::Text(refCount.c_str());
						}
						if (col == 4)
						{
							// get info about the physical page before displaying it
							PhysicalMemoryPage* physicalPage = job->virtualMemoryPages[row]->physicalMemoryPage;
							if (physicalPage)
							{
								auto physicalPageInfo = fmt::format("{} - {:#x}", physicalPage->index,
								                                    physicalPage->physicalAddress);
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

void MemoryManagerGuiContext::ShowPhysicalMemoryControl() const
{
	ImGui::BeginChild(6, ImVec2(300 * dpiScaleFactor, 300 * dpiScaleFactor), true);
	ImGui::Text("Physical Memory");
	ImGui::Separator();

	ImGuiTableFlags tableFlags = 0;
	tableFlags |= ImGuiTableFlags_Borders;
	tableFlags |= ImGuiTableFlags_RowBg;

	if (ImGui::BeginTable("table2", 5, tableFlags))
	{
		// setup headers/columns
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
			// for each row
			for (size_t row = 0; row < memoryManager->PHYSICAL_PAGES; row++)
			{
				ImGui::TableNextRow();
				// for each col
				for (size_t col = 0; col < 5; col++)
				{
					ImGui::TableSetColumnIndex(col);
					ImGui::AlignTextToFramePadding();
					// display data based on which column were in
					if (memoryManager && memoryManager->memoryManager->jobManager->jobs->length > 0)
					{
						VirtualMemoryPage* virtualPage = memoryManager->memoryManager->physicalMemoryPages[row]->
							virtualMemoryPage;
						if (col == 0)
						{
							string addr = fmt::format("{:#x}", physicalAddress);
							ImGui::Text(addr.c_str());
							physicalAddress += memoryManager->PAGE_SIZE;
						}
						if (col == 1)
						{
							string index = to_string(
								memoryManager->memoryManager->physicalMemoryPages[row]->index);
							ImGui::Text(index.c_str());
						}
						if (col == 2)
						{
							if (virtualPage)
							{
								size_t jobId;
								// loop through all the jobs
								for (size_t jobIdx = 0; jobIdx < memoryManager->memoryManager->jobManager->jobs->length;
								     jobIdx++)
								{
									// loop through all the pages in the job
									auto job = static_cast<Job*>(memoryManager->memoryManager->jobManager->jobs->
										getByIndex(
											memoryManager->memoryManager->jobManager->jobs, jobIdx));
									for (size_t pgIdx = 0; pgIdx < memoryManager->VIRTUAL_PAGES; pgIdx++)
									{
										// if the virtual page equals our physical page, that is the job we are looking for
										if (job->virtualMemoryPages[pgIdx] == virtualPage)
										{
											jobId = job->id;
											break;
										}
									}
								}
								ImGui::Text(to_string(jobId).c_str());
							}
						}
						if (col == 3)
						{
							if (virtualPage)
							{
								ImGui::Text(to_string(virtualPage->index).c_str());
							}
						}
						if (col == 4)
						{
							if (virtualPage)
							{
								// dispaly information about virtual page
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
