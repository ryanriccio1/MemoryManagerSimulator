#include "gtest/gtest.h"

extern "C" {
#include "MemoryManager.h"
}

using namespace std;

MemoryManager* setupTestMemoryManager()
{
	// create job
	auto memory_manager = new MemoryManager;
	memory_manager = setupMemoryManager(memory_manager, 16, 128, 1024);
	createJob(memory_manager, "test1", 1);
	createJob(memory_manager, "test2", 300);

	// perform test allocations
	for (size_t idx = 0; idx < 2000; idx++)
	{
		accessJob(memory_manager, 1, idx * 30 % 1024, FIFO);
		accessJob(memory_manager, 300, idx * 236 % 1024, LRU);
	}
	return memory_manager;
}

TEST(memory_manager, createJob_withValidData_returnsValidJob)
{
	const auto memory_manager = setupTestMemoryManager();
	const auto job = createJob(memory_manager, "test", 1234);
	EXPECT_TRUE(contains(memory_manager->jobManager->jobs, job));

	// make sure data values were set
	// convert to std::string so proper comparison can occur
	const string name = job->name;
	EXPECT_EQ(name, "test");
	EXPECT_EQ(job->id, 1234);
	delete memory_manager;
}

TEST(memory_manager, createJob_withExistingID_returnsNull)
{
	const auto memory_manager = setupTestMemoryManager();
	const auto job = createJob(memory_manager, "test", 1);
	const auto real_job = findJob(memory_manager, 1);
	EXPECT_EQ(job, nullptr);

	// make sure we did not touch the origin
	// convert to std::string so proper comparison can occur
	const string name = real_job->name;
	EXPECT_EQ(name, "test1");
	EXPECT_EQ(real_job->id, 1);
	delete memory_manager;
}

TEST(memory_manager, removeJob_withValidData_removeValidTest)
{
	const auto memory_manager = setupTestMemoryManager();
	const auto job1 = static_cast<Job*>(getByIndex(memory_manager->jobManager->jobs, 0));
	// keep track of all pages referenced by job before removing the job
	auto virtualPages = vector<VirtualMemoryPage*>();
	auto physicalPages = vector<PhysicalMemoryPage*>();
	for (size_t idx = 0; idx < memory_manager->VIRTUAL_PAGES; idx++)
	{
		virtualPages.push_back(job1->virtualMemoryPages[idx]);
		const PhysicalMemoryPage* tempPhysicalPage = virtualPages[idx]->physicalMemoryPage;
		if (tempPhysicalPage)
			physicalPages.push_back(virtualPages[idx]->physicalMemoryPage);
	}

	// remove job
	EXPECT_TRUE(removeJob(memory_manager, 1));

	// loop through all physical pages
	for (size_t idx = 0; idx < memory_manager->PHYSICAL_PAGES; idx++)
	{
		// if our physical page was in the removed job
		if (count(physicalPages.begin(), physicalPages.end(), memory_manager->physicalMemoryPages[idx]))
		{
			// make sure that it is no longer linking to a virtual page
			if (memory_manager->physicalMemoryPages[idx]->virtualMemoryPage)
			{
				FAIL() << "Not all physical pages were removed";
			}
		}
	}

	// loop through validVirtualPages and make sure no removed pages exist
	for (const auto virtualPage : virtualPages)
	{
		// if the virtual page is still in validVirtualPages
		if (contains(memory_manager->validVirtualPages, virtualPage))
		{
			FAIL() << "Virtual page was not removed from validVirtualPages";
		}
	}

	// loop through lruQueue, fifoQueue, free physical page list and make sure no pages from physicalPages exist
	for (const auto physicalPage : physicalPages)
	{
		if (!contains(memory_manager->freePhysicalPages, physicalPage))
			FAIL() << "Physical page not in free physical pages";
		if (contains(memory_manager->lruQueue->list, physicalPage))
			FAIL() << "lruQueue still contains freed physical page";
		if (contains(memory_manager->fifoQueue->list, physicalPage))
			FAIL() << "fifoQueue still contains freed physical page";
	}
	delete memory_manager;
}

TEST(memory_manager, removeJob_withInvalidID_returnsFalse)
{
	const auto memory_manager = setupTestMemoryManager();
	EXPECT_FALSE(removeJob(memory_manager, 1234));
	delete memory_manager;
}

TEST(memory_manager, accessJob_withValidData_returnsValidAddress)
{
	const auto memory_manager = setupTestMemoryManager();
	// clear all memory
	removeJob(memory_manager, 1);
	removeJob(memory_manager, 300);

	createJob(memory_manager, "test", 0);
	createJob(memory_manager, "test", 300);
	EXPECT_EQ((int)accessJob(memory_manager, 0, 0x21, LRU), 0x01);
	EXPECT_EQ((int)accessJob(memory_manager, 0, 0x05, LRU), 0x15);
	EXPECT_EQ((int)accessJob(memory_manager, 300, 0x05, LRU), 0x25);
	delete memory_manager;
}

TEST(memory_manager, accessJob_withInvalidAddress_returnsNull)
{
	const auto memory_manager = setupTestMemoryManager();
	EXPECT_EQ(accessJob(memory_manager, 300, 0xffff, LRU), nullptr);
	delete memory_manager;
}

TEST(memory_manager, accessJob_withInvalidID_returnsNull)
{
	const auto memory_manager = setupTestMemoryManager();
	EXPECT_EQ(accessJob(memory_manager, 1234, 0x21, LRU), nullptr);
	delete memory_manager;
}

TEST(memory_manager, findJob_withValidData_returnsJob)
{
	const auto memory_manager = setupTestMemoryManager();
	EXPECT_TRUE(findJob(memory_manager, 300));
	delete memory_manager;
}

TEST(memory_manager, findJob_withInvalidID_returnsNull)
{
	const auto memory_manager = setupTestMemoryManager();
	EXPECT_FALSE(findJob(memory_manager, 1234));
	delete memory_manager;
}

TEST(memory_manager, getFreePage_withLRUMethod_returnsFreePage)
{
	const auto memory_manager = setupTestMemoryManager();
	EXPECT_EQ(getFreePage(memory_manager, LRU)->index, 0); // calculated index of next free page
	delete memory_manager;
}

TEST(memory_manager, getFreePage_withLFUMethod_returnsFreePage)
{
	const auto memory_manager = setupTestMemoryManager();
	EXPECT_EQ(getFreePage(memory_manager, LFU)->index, 0); // calculated index of next free page
	delete memory_manager;
}

TEST(memory_manager, getFreePage_withFIFOMethod_returnsFreePage)
{
	const auto memory_manager = setupTestMemoryManager();
	EXPECT_EQ(getFreePage(memory_manager, FIFO)->index, 0); // calculated index of next free page
	delete memory_manager;
}

TEST(memory_manager, uint64log2_withInt_returnsValidLog2)
{
	const auto ground_truth = static_cast<int>(log2(100));
	EXPECT_EQ(uint64log2(100), ground_truth);
}
