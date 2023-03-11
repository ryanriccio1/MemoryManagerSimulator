#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <string>
#include <cmath>

#include "MemoryManagerGuiContext.h"
#include "MemoryManagerWrapper.h"
#include "ConsolaTTF.h"

#pragma warning(disable : 4996)

MemoryManagerGuiContext::MemoryManagerGuiContext(GLFWwindow *window, const char *glsl_version) : ImGuiDataContext(window, glsl_version), window(window)
{
	MemoryManagerConstants::initializeConsts();

	float xScale, yScale;
	glfwGetWindowContentScale(window, &xScale, &yScale);
	dpiScaleFactor = xScale;

	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(ConsolaTTF_compressed_data_base85, 13 * dpiScaleFactor);
	largeFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(ConsolaTTF_compressed_data_base85, 26 * dpiScaleFactor);
	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowRounding = 6.0f;
	ImGui::GetStyle().ScaleAllSizes(dpiScaleFactor);

	// windowFlags = 0;
	// windowFlags |= ImGuiWindowFlags_NoTitleBar;
	// windowFlags |= ImGuiWindowFlags_NoMove;
	// windowFlags |= ImGuiWindowFlags_NoResize;
	// windowFlags |= ImGuiWindowFlags_NoCollapse;
}

void MemoryManagerGuiContext::Update()
{

	// render your GUI
	static int addressToAccess;
	static int currentJobId;
	static int currentJobIdx;
	static ReplacementMethod method;

	// UpdateWindowSize();
	ImGui::Begin("Memory Manager Simulator"); // , nullptr, windowFlags);              // Create a window called "Hello, world!" and append into it.
	if (ImGui::Button("Change Settings") || firstRun)
	{
		currentJobId = -1;
		currentJobIdx = -1;
		method = LRU;
		ImGui::OpenPopup("Set Constants");
	}

	ShowConstantEditor();
	ShowCreateJobControl();
	ImGui::SameLine();
	ShowJobOperationControl(currentJobIdx, currentJobId, method, addressToAccess);
	ShowVirtualMemoryControl(currentJobIdx, currentJobId);
	ImGui::SameLine();
	ShowPhysicalMemoryControl();

	ImGui::BeginChild(7, ImVec2(300 * dpiScaleFactor, 150 * dpiScaleFactor), true);
	ImGui::Text("Instruction Inspection");
	ImGui::Separator();
	unsigned int testInstr = 63;
	ImGui::PushFont(largeFont);

	for (size_t idx = 0; idx < *MemoryManagerConstants::m_PAGE_BITS; idx++)
	{
		unsigned int tmp = testInstr << (sizeof(unsigned int) * 8 - *MemoryManagerConstants::m_INSTRUCTION_BITS + idx) >> ((sizeof(unsigned int) * 8) - 1);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), std::to_string(tmp).c_str());
	}

	for (size_t idx = 0; idx < *MemoryManagerConstants::m_OFFSET_BITS; idx++)
	{
		unsigned int tmp = testInstr << (sizeof(unsigned int) * 8 - *MemoryManagerConstants::m_PAGE_BITS + idx) >> ((sizeof(unsigned int) * 8) - 1);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), std::to_string(tmp).c_str());
	}

	ImGui::PopFont();

	ImGui::EndChild();

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::ShowDemoWindow();
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

	if (ImGui::BeginTable("table1", 3, tableFlags))
	{
		// Display headers so we can inspect their interaction with borders.
		// (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for detail
		ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
		ImGui::TableSetupColumn("Addr", columnFlags, 45.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("IDX", columnFlags, 25.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Virtual", columnFlags, 0);
		ImGui::TableHeadersRow();

		int physicalAddress = 0;
		for (size_t row = 0; row < *MemoryManagerConstants::m_PHYSICAL_PAGES; row++)
		{
			ImGui::TableNextRow();
			for (size_t col = 0; col < 3; col++)
			{
				ImGui::TableSetColumnIndex(col);
				ImGui::AlignTextToFramePadding();
				if (memoryManager && memoryManager->memoryManager->jobManager->jobs->length > 0)
				{
					if (col == 0)
					{
						std::string addr = fmt::format("{:#x}", physicalAddress);
						ImGui::Text(addr.c_str());
						physicalAddress += *MemoryManagerConstants::m_PAGE_SIZE;
					}
					if (col == 1)
					{
						std::string index = std::to_string(memoryManager->memoryManager->physicalMemoryPages[row]->index);
						ImGui::Text(index.c_str());
					}
					if (col == 2)
					{
						VirtualMemoryPage *virtualPage = memoryManager->memoryManager->physicalMemoryPages[row]->virtualMemoryPage;
						if (virtualPage)
						{
							size_t jobId;
							for (size_t jobIdx = 0; jobIdx < memoryManager->memoryManager->jobManager->jobs->length; jobIdx++)
							{
								Job *job = (Job *)memoryManager->memoryManager->jobManager->jobs->getByIndex(memoryManager->memoryManager->jobManager->jobs, jobIdx);
								for (size_t pgIdx = 0; pgIdx < *MemoryManagerConstants::m_VIRTUAL_PAGES; pgIdx++)
								{
									if (job->pmt->virtualMemoryPages[pgIdx] == virtualPage)
										jobId = job->id;
								}
							}
							std::string physicalPageInfo = "Job " + std::to_string(jobId) + ": " + std::to_string(virtualPage->index);
							ImGui::Text(physicalPageInfo.c_str());
						}
					}
				}
				else
					ImGui::Text("");
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
		ImGui::TableSetupColumn("Valid", columnFlags, 40.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Ref", columnFlags, 40.0f * dpiScaleFactor);
		ImGui::TableSetupColumn("Physical", columnFlags, 0);
		ImGui::TableHeadersRow();

		Job *job = nullptr;
		if (memoryManager && currentJobId >= 0)
			job = (Job *)memoryManager->memoryManager->jobManager->jobs->getByIndex(memoryManager->memoryManager->jobManager->jobs, currentJobIdx);
		int virtualAddress = 0;
		for (size_t row = 0; row < *MemoryManagerConstants::m_VIRTUAL_PAGES; row++)
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
						virtualAddress += *MemoryManagerConstants::m_PAGE_SIZE;
					}
					if (col == 1)
					{
						std::string index = std::to_string(job->pmt->virtualMemoryPages[row]->index);
						ImGui::Text(index.c_str());
					}
					if (col == 2)
					{
						std::string valid = std::to_string(job->pmt->virtualMemoryPages[row]->valid);
						ImGui::Text(valid.c_str());
					}
					if (col == 3)
					{
						std::string refCount = std::to_string(job->pmt->virtualMemoryPages[row]->refCount);
						ImGui::Text(refCount.c_str());
					}
					if (col == 4)
					{
						PhysicalMemoryPage *physicalPage = job->pmt->virtualMemoryPages[row]->physicalMemoryPage;
						if (physicalPage)
						{
							std::string physicalPageInfo = std::to_string(physicalPage->index) + " - " + fmt::format("{:#x}", physicalPage->physicalAddress);
							ImGui::Text(physicalPageInfo.c_str());
						}
						else
							ImGui::Text("");
					}
				}
				else
					ImGui::Text("");
			}
		}
		ImGui::EndTable();
	}

	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowConstantEditor()
{
	// Always center this window when appearing
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Set Constants", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Constants:");
		ImGui::Separator();

		ImGui::InputInt("Page Size", (int *)MemoryManagerConstants::m_PAGE_SIZE, 16, 128);
		if (*MemoryManagerConstants::m_PAGE_SIZE < 1)
		{
			*MemoryManagerConstants::m_PAGE_SIZE = 16;
		}
		if (*MemoryManagerConstants::m_PAGE_SIZE % 16 != 0)
		{
			*MemoryManagerConstants::m_PAGE_SIZE -= *MemoryManagerConstants::m_PAGE_SIZE % 16;
		}
		if (*MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE < *MemoryManagerConstants::m_PAGE_SIZE ||
			*MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE < *MemoryManagerConstants::m_PAGE_SIZE)
		{
			*MemoryManagerConstants::m_PAGE_SIZE = std::min(*MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE, *MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE);
		}

		ImGui::InputInt("Physical Memory Size", (int *)MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE, 16, 128);
		if (*MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE < 1)
		{
			*MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE = 16;
		}
		if (*MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE % 16 != 0)
		{
			*MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE -= *MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE % 16;
		}

		ImGui::InputInt("Virtual Memory Size", (int *)MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE, 16, 128);
		if (*MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE < 1)
		{
			*MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE = 16;
		}
		if (*MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE % 16 != 0)
		{
			*MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE -= *MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE % 16;
		}

		ImGui::Text("Virtual Pages:\t    %lld", *MemoryManagerConstants::m_VIRTUAL_PAGES);
		ImGui::Text("Physical Pages:\t   %lld", *MemoryManagerConstants::m_PHYSICAL_PAGES);
		ImGui::Text("Offset Bits:\t      %lld", *MemoryManagerConstants::m_OFFSET_BITS);
		ImGui::Text("Page Bits:\t        %lld", *MemoryManagerConstants::m_PAGE_BITS);
		ImGui::Text("Instruction Bits:\t %lld", *MemoryManagerConstants::m_INSTRUCTION_BITS);

		UpdateConstantValues();

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			firstRun = false;
			ImGui::CloseCurrentPopup();
			memoryManager = std::make_shared<MemoryManagerSimulatorWrapper>();
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
		strcpy_s(jobName, "");
	}

	ImGui::EndChild();
}

void MemoryManagerGuiContext::ShowJobOperationControl(int &currentJobIdx, int &currentJobId, ReplacementMethod &method, int &addressToAccess)
{
	ImGui::BeginChild(4, ImVec2(300 * dpiScaleFactor, 175 * dpiScaleFactor), true);
	ImGui::Text("Job Operations");
	ImGui::Separator();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Job:");
	ImGui::SameLine(ImGui::GetContentRegionAvail().x - 185 * dpiScaleFactor);

	if (memoryManager)
		ImGui::BeginDisabled(memoryManager->memoryManager->jobManager->jobs->length < 1);
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
		ImGui::BeginDisabled(memoryManager->memoryManager->jobManager->jobs->length < 1);
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

	ImGui::BeginDisabled(currentJobIdx == -1);
	ImGui::InputInt("##label_addressToAccess", &addressToAccess, 0, 0, inputFlags);

	ImGui::Spacing();
	if (ImGui::Button("Access Job", ImVec2(ImGui::GetContentRegionAvail().x, 30 * dpiScaleFactor)))
	{
		memoryManager->m_accessJob(currentJobId, addressToAccess, method);
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
					std::string label = jobIdString + "\t|\t" + jobNameString;
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

void MemoryManagerGuiContext::UpdateWindowSize()
{
	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);

	windowSize = ImVec2(static_cast<float>(screen_width), static_cast<float>(screen_height));

	ImGui::SetNextWindowSize(windowSize);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
}

void MemoryManagerGuiContext::UpdateConstantValues() const
{
	*MemoryManagerConstants::m_OFFSET_BITS = (uint64_t)log2(*MemoryManagerConstants::m_PAGE_SIZE);
	*MemoryManagerConstants::m_INSTRUCTION_BITS = (uint64_t)log2(*MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE);
	*MemoryManagerConstants::m_PAGE_BITS = *MemoryManagerConstants::m_INSTRUCTION_BITS - *MemoryManagerConstants::m_OFFSET_BITS;
	if (*MemoryManagerConstants::m_PAGE_SIZE != 0)
	{
		*MemoryManagerConstants::m_VIRTUAL_PAGES = *MemoryManagerConstants::m_VIRTUAL_MEMORY_SIZE / *MemoryManagerConstants::m_PAGE_SIZE;
		*MemoryManagerConstants::m_PHYSICAL_PAGES = *MemoryManagerConstants::m_PHYSICAL_MEMORY_SIZE / *MemoryManagerConstants::m_PAGE_SIZE;
	}
	else
	{
		*MemoryManagerConstants::m_VIRTUAL_PAGES = 0;
		*MemoryManagerConstants::m_PHYSICAL_PAGES = 0;
	}
}
