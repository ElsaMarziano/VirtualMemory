#include VirtualMemory.h
#include MemoryConstants.h
#include PhysicalMemory.h

void VMinitialize ()
{
  PMwrite (0, 0);
//  Put root at 0
//  for (uint64_t i = 1; i < NUM_FRAMES; i++) {
//    PMwrite(i, 0);
//  }
}

int VMread (uint64_t virtualAddress, word_t *value)
{
  uint64_t physicalAddress = mapAddress (virtualAddress);
  PMread (physicalAddress, value);
  return SUCCESS;
}

int VMwrite (uint64_t virtualAddress, word_t value)
{
  uint64_t physicalAddress = mapAddress (virtualAddress);
  PMwrite (physicalAddress, value);
  return SUCCESS;
}

uint64_t mapAddress (uint64_t virtualAddress)
{
  const offset_mask = (1 << OFFSET_WIDTH) - 1;
  word_t address = 0;

  for (int i = TABLE_DEPTH; i >= 0; i--)
  {
    uint64_t pageIndex = (virtualAddress >> (OFFSET_WIDTH * i)) & offset_mask;
    if (i == 0) return address * PAGE_SIZE + pageIndex;
    PMread (address * PAGE_SIZE + pageIndex, address);
    if (address == 0) // Page fault
    {
      const f1 = findFrame (address * PAGE_SIZE + pageIndex);
      if (i > 1) PMWrite (f1, 0); // ???
      PMwrite (address * PAGE_SIZE + pageIndex, f1);
      address = f1;
    }
  }
  return address;
}

uint64_t findFrame (uint64_t pageIndex)
{
  uint64_t maxFrameIndex = 0;
  const frameIndex = dfs (0, 0, &maxFrameIndex);
  if (frameIndex == -1)
  {
    if (maxFrameIndex + 1 < NUM_PAGES)
    {
      frameIndex = maxFrameIndex
    } else {
        for(uint64_t i = 0; i < NUM_FRAMES; i++)
        {

//          PMevict (i, i);
        }
    }
  }
  PMrestore (frameIndex, pageIndex)
  return frameIndex;
}

uint64_t dfs (uint64_t i, int depth, uint64_t *maxFrameIndex)
{
  if (depth >= TABLES_DEPTH) return -1;
  for (uint64_t j = 0; j < PAGE_SIZE; j++)
  {
    if (i * PAGE_SIZE + j > &maxFrameIndex) *maxFrameIndex = i * PAGE_SIZE + j;
    word_t value;
    PMread (i * PAGE_SIZE + j, value);
    if (value != 0)
    {
      const nextAddress = dfs (value, depth + 1);
      if (nextAddress != -1)
      {
        PMwrite (i * PAGE_SIZE + j, 0);
        return nextAddress;
      }
    }
  }
  return i * PAGE_SIZE;;
}
