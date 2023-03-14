#include "pch.h"

extern "C"
{
#include "MemoryManager.h"
}
//
//TEST(TestCaseName, TestName)
//{
//  EXPECT_EQ(1, 1);
//  EXPECT_TRUE(true);
//}
//
//TEST(memory_manager, createJob_withValidData_returnsValidJob)
//{
//  EXPECT_EQ(1, 1);
//  EXPECT_TRUE(true);
//}
//TEST(memory_manager, createJob_withExistingID_returnsNull)
//{
//}
//
//TEST(memory_manager, removeJob_withValidData_removeValidTest)
//{
//  // create job
//  MemoryManager *test = (MemoryManager *)malloc(sizeof(MemoryManager));
//  setupMemoryManager(test, 16, 128, 1024);
//  Job *job1 = createJob(test, "test1", 1);
//  Job *job2 = createJob(test, "test2", 300);
//  for (size_t idx = 0; idx < 2000; idx++)
//  {
//    accessJob(test, 1, idx * 30 % 1024, FIFO);
//    accessJob(test, 300, idx * 236 % 1024, LRU);
//  }
//
//  // perform test memory allocations (accesses)
//
//  VirtualMemoryPage *virtualPages[] = {(VirtualMemoryPage *)malloc(test->VIRTUAL_PAGES * sizeof(VirtualMemoryPage *))};
//  PhysicalMemoryPage *physicalPages[] = {(PhysicalMemoryPage *)malloc(test->PHYSICAL_PAGES * sizeof(PhysicalMemoryPage *))};
//  for (size_t idx = 0; idx < test->VIRTUAL_PAGES; idx++)
//  {
//    virtualPages[idx] = job1->virtualMemoryPages[idx];
//    const PhysicalMemoryPage *tempPhysicalPage = virtualPages[idx]->physicalMemoryPage;
//    physicalPages[tempPhysicalPage->index] = virtualPages[idx]->physicalMemoryPage;
//  }
//  // remove job
//  removeJob(test, 1);
//
//  // loop through ALL physical pages and make sure none of them link to a removed virtual page
//  for (size_t idx = 0; idx < test->PHYSICAL_PAGES; idx++)
//  {
//    if (test->physicalMemoryPages[idx] == physicalPages[idx])
//    {
//      FAIL() << "Not all physical pages were removed";
//    }
//  }
//  // loop through validVirtualPages and make sure no removed pages exist
//
//  // loop through lruQueue, fifoQueue, free physical page list and make sure no pages from physicalPages exist
//  // bool found = false;
//  // loop
//  //		if it exists
//  //			found = true
//  //			break
//  // EXPECT_FALSE(found)
//  EXPECT_FALSE(found);
//}
//
//TEST(memory_manager, removeJob_withInvalidID_returnsNull)
//{
//}
//
//TEST(memory_manager, accessJob_withValidData_returnsValidAddress)
//{
//}
//
//TEST(memory_manager, accessJob_withInvalidAddress_returnsNull)
//{
//}
//
//TEST(memory_manager, accessJob_withInvalidID_returnsNull)
//{
//}
//
//TEST(memory_manager, findJob_withValidData_returnsJob)
//{
//}
//TEST(memory_manager, findJob_withInvalidID_returnsNull)
//{
//}
//TEST(memory_manager, getFreePage_withLRUMethod_returnsFreePage)
//{
//}
//TEST(memory_manager, getFreePage_withLFUMethod_returnsFreePage)
//{
//}
//TEST(memory_manager, getFreePage_withFIFOMethod_returnsFreePage)
//{
//}
//
//TEST(memory_manager, uint64log2_withInt_returnsValidLog2)
//{
//}