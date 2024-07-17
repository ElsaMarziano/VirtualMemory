#include "VirtualMemory.h"
#include "MemoryConstants.h"
#include "PhysicalMemory.h"
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <cmath>

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
        if (i == TABLES_DEPTH && START_OFFSET != 0) {
            current_offset = (virtualAddress >> (VIRTUAL_ADDRESS_WIDTH - START_OFFSET)) &
                             ((1 << START_OFFSET) - 1);
        } else {
            current_offset = (virtualAddress >> (OFFSET_WIDTH * i)) &
                             offset_mask;
        }
        if (i == 0) return prev * PAGE_SIZE + current_offset;
        PMread(prev * PAGE_SIZE + current_offset, &address);
//        printf("ss:%d\n", address);
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
//            printf("adress2:%d\n", prev * PAGE_SIZE + current_offset);
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
//    printf("fine frame:\n");
    word_t maxFrameIndex = 0;
    uint64_t page = 0;
    uint64_t maxCycle = 0;
    word_t cycleFrame = 0;
    uint64_t pageToEvict = 0;
    uint64_t cycleParent = 0;
//  TODO Link frameIndex to parent
    auto frameIndex = dfs(0, 0, &maxFrameIndex, prev, 0, page,
                          swapIn, &cycleFrame, &maxCycle, &pageToEvict, &cycleParent);

    if (frameIndex == -1) {
        if (maxFrameIndex + 1 < NUM_FRAMES) {
            frameIndex = maxFrameIndex + 1;

            for (uint64_t i = 0; i < PAGE_SIZE; i++) {
                PMwrite(frameIndex * PAGE_SIZE + i, 0);
            }
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

int countBits(uint64_t number) {
    if (number == 0) return 1;
    return static_cast<int>(std::log2(number)) + 1;
}

word_t dfs(uint64_t i, int depth, word_t *maxFrameIndex, word_t
previousFrame, word_t fatherFrameIndex, uint64_t page, uint64_t swapIn,
           word_t *cycleFrame, uint64_t *maxCycle, uint64_t *pageToEvict, uint64_t *cycleParent) {
    word_t currentFrame = 0;
    bool isAlZeros = true;
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
//        PMwrite(fatherFrameIndex, 0);
//        printf("cycle=%d, %d\n", *cycleFrame, *maxCycle);
        return -1;
    }
    // Iterate over all children nodes
    for (int j = 0; j < PAGE_SIZE; j++) {
        PMread(i * PAGE_SIZE + j, &currentFrame);
        if (currentFrame > *maxFrameIndex) {
            *maxFrameIndex = currentFrame;
        }
        if (currentFrame != 0) {
            isAlZeros = false;
            // Update the page number
            int bits = countBits(j);
            page = ((page) << bits) | j;
            std::cout << page << " " << j << " " << bits << std::endl;
            // Recursively call dfs for the next level
            auto nextAddress = dfs(currentFrame, depth + 1, maxFrameIndex,
                                   previousFrame, i * PAGE_SIZE + j, page, swapIn,
                                   cycleFrame, maxCycle, pageToEvict, cycleParent);
            // If a valid address is returned, propagate it upwards
            if (nextAddress != -1) {
                return nextAddress;
            }
//            else return -1;
        }
    }
    if (!isAlZeros) {
        return -1;
    }
    if (i == previousFrame) {
//        printf("lll\n");
        return -1;
    }
//    printf("end\n");
    PMwrite(fatherFrameIndex, 0);

    return i;
}

void VMinitialize() {
    for (int i = 0; i < PAGE_SIZE; i++) {
        PMwrite(i, 0);

    }
}

int VMread(uint64_t virtualAddress, word_t *value) {
    uint64_t physicalAddress = mapAddress(virtualAddress);
//    printf("addddd=%d", physicalAddress);
    PMread(physicalAddress, value);
    return 1;
}

int VMwrite(uint64_t virtualAddress, word_t value) {
    uint64_t physicalAddress = mapAddress(virtualAddress);
    PMwrite(physicalAddress, value);
    return 1;
}
