#include "VirtualMemory.h"
#include "MemoryConstants.h"
#include "PhysicalMemory.h"
#include <algorithm>
#include <cmath>
#include <iostream>
//#include <iostream>

#define START_OFFSET (VIRTUAL_ADDRESS_WIDTH%OFFSET_WIDTH)

uint64_t mapAddress(uint64_t virtualAddress);


word_t findFrame(uint64_t pageIndex, word_t prev, word_t parentFrameIndex, uint64_t
swapIn);

uint64_t mapAddress(uint64_t virtualAddress) {
    auto offset_mask = (1 << OFFSET_WIDTH) - 1;
    word_t address = 0;
    word_t prev = 0;

    word_t current_offset;
    for (int i = TABLES_DEPTH; i >= 0; i--) {
        // Calculate offset for the first part of the address
//        TODO Check this works with other examples
        if (i == TABLES_DEPTH && START_OFFSET != 0) {
            current_offset = (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - START_OFFSET)) &
                             ((1 << START_OFFSET) - 1);
        } else {
            current_offset = (virtualAddress >> (OFFSET_WIDTH * i)) &
                             offset_mask;
        }
        if (i == 0) return prev * PAGE_SIZE + current_offset;
        PMread(prev * PAGE_SIZE + current_offset, &address);
        if (address == 0) // Page fault
        {
            // Define a bit mask to keep the first (number of bits - offset) bits
            uint16_t bitMask = (1 << (VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH)) - 1;
            word_t f1 = findFrame(prev * PAGE_SIZE + current_offset, prev, prev * PAGE_SIZE,
                                  virtualAddress & bitMask);
            if (i > 1) {
                for (int j = 0; j < PAGE_SIZE; j++) {
                    PMwrite(f1 * PAGE_SIZE + j, 0);
                }
            }
            PMwrite(prev * PAGE_SIZE + current_offset, f1);
            prev = f1;
        } else {
            prev = address;
        }
    }
    return address;
}

word_t dfs(uint64_t i, int depth, word_t *maxFrameIndex, word_t
previousFrame, word_t fatherFrameIndex, uint64_t page, uint64_t swapIn,
           word_t *cycleFrame, uint64_t *maxCycle, uint64_t *pageToEvict, uint64_t *cycleParent);


word_t findFrame(uint64_t pageIndex, word_t prev, word_t parentFrameIndex, uint64_t
swapIn) {
    word_t maxFrameIndex = 0;
    uint64_t page = 0;
    uint64_t maxCycle = 0;
    word_t cycleFrame = 0;
    uint64_t pageToEvict = 0;
    uint64_t cycleParent = 0;
//    Find frame index to create table
    auto frameIndex = dfs(0, 0, &maxFrameIndex, prev, 0, page,
                          swapIn, &cycleFrame, &maxCycle, &pageToEvict, &cycleParent);
//    Didn't find a frame
    if (frameIndex == -1) {
//        Either create it
        if (maxFrameIndex + 1 < NUM_FRAMES) {
            frameIndex = maxFrameIndex + 1;
            for (uint64_t i = 0; i < PAGE_SIZE; i++) {
                PMwrite(frameIndex * PAGE_SIZE + i, 0);
            }
//            If there's already the max number of frames, evict one
        } else {
            frameIndex = cycleFrame;
            PMwrite(cycleParent, 0);
//            PMevict(frameIndex * PAGE_SIZE, page); //check if needs the *Page_Size
            PMevict(frameIndex, pageToEvict);
        }
    }
    PMrestore(frameIndex, pageIndex);
    return frameIndex;
}


word_t dfs(uint64_t i, int depth, word_t *maxFrameIndex, word_t
previousFrame, word_t fatherFrameIndex, uint64_t page, uint64_t swapIn,
           word_t *cycleFrame, uint64_t *maxCycle, uint64_t *pageToEvict, uint64_t *cycleParent) {
    word_t currentFrame = 0;
    bool isAllZeros = true;
    // Base case: if we reached the maximum depth
    if (depth >= TABLES_DEPTH) {
        uint64_t cycle = std::min(
                static_cast<uint64_t>(NUM_PAGES -
                                      static_cast<uint64_t>(std::abs(static_cast<int64_t>(swapIn - page)))),
                static_cast<uint64_t>(std::abs(static_cast<int64_t>(swapIn - page))));
        if (cycle > *maxCycle) {
            *maxCycle = cycle;
            *cycleFrame = i;
            *pageToEvict = page;
            *cycleParent = fatherFrameIndex;
        }
        return -1;
    }
    // Iterate over all children nodes
    int bits;
    if (depth == TABLES_DEPTH - 1 && START_OFFSET != 0) {
        bits = START_OFFSET;
    } else {
        bits = OFFSET_WIDTH;
    }
    for (int j = 0; j < PAGE_SIZE; j++) {
        PMread(i * PAGE_SIZE + j, &currentFrame);
        if (currentFrame > *maxFrameIndex) {
            *maxFrameIndex = currentFrame;
        }
        if (currentFrame != 0) {
            isAllZeros = false;
            // Update the page number
            auto nextPage = ((page) << bits) | j;
            std::cout << "next " << nextPage << std::endl;
            // Recursively call dfs for the next level
            auto nextAddress = dfs(currentFrame, depth + 1, maxFrameIndex,
                                   previousFrame, i * PAGE_SIZE + j, nextPage, swapIn,
                                   cycleFrame, maxCycle, pageToEvict, cycleParent);
            // If a valid address is returned, propagate it upwards
            if (nextAddress != -1) {
                return nextAddress;
            }
//            else return -1;
        }
    }
    if (!isAllZeros) {
        return -1;
    }
    if (i == previousFrame) {
        return -1;
    }

    PMwrite(fatherFrameIndex, 0);

    return i;
}

void VMinitialize() {
    for (int i = 0; i < PAGE_SIZE; i++) {
        PMwrite(i, 0);

    }
}

int VMread(uint64_t virtualAddress, word_t *value) {
    if (virtualAddress > (1ULL << VIRTUAL_ADDRESS_WIDTH) - 1 || value == nullptr) return 0;
    uint64_t physicalAddress = mapAddress(virtualAddress);
    PMread(physicalAddress, value);
    return 1;
}

int VMwrite(uint64_t virtualAddress, word_t value) {
    if (virtualAddress > (1ULL << VIRTUAL_ADDRESS_WIDTH) - 1) return 0;
    uint64_t physicalAddress = mapAddress(virtualAddress);
    PMwrite(physicalAddress, value);
    return 1;
}
