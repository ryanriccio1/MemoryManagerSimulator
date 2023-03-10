#include "pch.h"

extern "C"
{
#include "MemoryManager.h"
}

TEST(TestCaseName, TestName)
{
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

TEST(memory_manager, createJob_withValidData_createsValidTest)
{
	EXPECT_EQ(1, 1);
	EXPECT_TRUE(true);
}
//TEST(memory_manager, removeJob_withValidData_removeValidTest)
//{
//	// create job
//	MemoryManager* Test = (sizeof(MemoryManager));
//	setupMemoryManager(Test);
//	createJob(Test, "test1", 1);
//	createJob(Test,"test2", 300);
//	for (size_t idx = 0; idx < 2000; idx++)
//	{
//		accessJob(Test, 1, idx * 30 % 1024, FIFO);
//		accessJob(Test, 300, idx * 236 % 1024, LRU);
//	}
//	
//	// perform test memory allocations (accesses)
//	
//	VirtualMemoryPage* virtualPages[VIRTUAL_PAGES];
//	PhysicalMemoryPage* physicalPages[PHYSICAL_PAGES];
//	for (size_t idx = 0; idx < VIRTUAL_PAGES; idx++)
//	{
//		virtualPages[idx] = Job->pmt->virtualMemoryPages[idx];
//		const PhysicalMemoryPage* tempPhysicalPage = virtualPages[idx]->physicalMemoryPage;
//		physicalPages[tempPhysicalPage->index] = virtualPages[idx]->physicalMemoryPage;
//	}
//	// remove job
//	// loop through ALL physical pages and make sure none of them link to a removed virtual page
//	// loop through validVirtualPages and make sure no removed pages exist
//	// loop through lruQueue, fifoQueue, free physical page list and make sure no pages from physicalPages exist
//	// bool found = false;
//	// loop
//	//		if it exists
//	//			found = true
//	//			break
//	// EXPECT_FALSE(found)
//	EXPECT_FALSE(found);
//	
//}
//

