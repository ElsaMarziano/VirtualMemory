#include VirtualMemory.h
#include MemoryConstants.h
#include PhysicalMemory.h

void VMinitialize() {
}

int VMread(uint64_t virtualAddress, word_t* value) {
  PMread(???, value);
  return SUCCESS;
}

int VMwrite(uint64_t virtualAddress, word_t value) {
  PMwrite(???, value);
  return SUCCESS;
}

