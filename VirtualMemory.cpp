#include "VirtualMemory.h"
#include "MemoryConstants.h"
#include "PhysicalMemory.h"
#include <cstdint>


uint64_t mapAddress (uint64_t virtualAddress)
{
  auto offset_mask = (1 << OFFSET_WIDTH) - 1;
  word_t address = 0;

  for (int i = TABLE_DEPTH; i >= 0; i--)
  {
    uint64_t current_offset = (virtualAddress >> (OFFSET_WIDTH * i)) &
                              offset_mask;
    if (i == 0) return address * PAGE_SIZE + current_offset;
    PMread (address * PAGE_SIZE + current_offset, address);
    if (address == 0) // Page fault
    {
      auto f1 = findFrame (address * PAGE_SIZE + current_offset, address);
      if (i > 1)
      {
        for (int j = 0; j < NUM_PAGES; j++)
        {
          PMWrite (f1 + j, 0);
        }
      }
      PMwrite (address * PAGE_SIZE + current_offset, f1);
      address = f1;
    }
  }
}

uint64_t findFrame (uint64_t pageIndex, uint64_t parentFrameIndex
{
  uint64_t maxFrameIndex = 0;
//  TODO Link frameIndex to parent
  auto frameIndex = dfs (0, 0, &maxFrameIndex, parentFrameIndex);
  if (frameIndex == -1)
  {
    if (maxFrameIndex + 1 < NUM_PAGES)
    {
      frameIndex = maxFrameIndex;
      for (uint64_t i = 0; i < NUM_PAGES; i++)
      {
        PMwrite (frameIndex + i, 0);
      }
    }
    else
    {
      for (uint64_t i = 0; i < NUM_FRAMES; i++)
      {

//          PMevict (i, i);
      }
    }
  }
  PMrestore (frameIndex, pageIndex)
  return frameIndex;
}

uint64_t dfs (uint64_t i, int depth, uint64_t *maxFrameIndex, uint64_t
previousFrame, uint64_t fatherFrameIndex)
{
  word_t currentFrame = 0;
  if (depth >= TABLES_DEPTH) return -1;
  for (uint64_t j = 0; j < PAGE_SIZE; j++)
  {
    PMread (i * PAGE_SIZE + j, &currentFrame);
    if (currentFrame > *maxFrameIndex)
    {
      *maxFrameIndex = currentFrame;
    }
    if (currentFrame != 0 || i * PAGE_SIZE == previousFrame)
    {
      auto nextAddress = dfs (currentFrame, depth + 1, maxFrameIndex,
                               previousFrame, i * PAGE_SIZE + j);
      if (nextAddress != -1)
      {
        return nextAddress;
      }
      else return -1;
    }
  }
  PMwrite (fatherFrameIndex, 0);
  return i * PAGE_SIZE;
}

void VMinitialize ()
{
  for (int i = 0; i < NUM_PAGES; i++)
  {
    PMwrite (i, 0);

  }
}

int VMread (uint64_t virtualAddress, word_t *value)
{
  uint64_t physicalAddress = mapAddress (virtualAddress);
  PMread (physicalAddress, value);
  return 1;
}

int VMwrite (uint64_t virtualAddress, word_t value)
{
  uint64_t wordIndex = mapAddress (virtualAddress);
  PMwrite (wordIndex, value);
  return 1;
}
