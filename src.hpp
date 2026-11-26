#ifndef BLOCK_ALLOCATOR_SRC_HPP
#define BLOCK_ALLOCATOR_SRC_HPP

#include <cstddef>
#include <functional>
#include <map>
#include <utility>
#include <vector>

int* getNewBlock(int n);
void freeBlock(const int* block, int n);

class Allocator {
    static constexpr std::size_t intsPerBlock = 4096 / sizeof(int);

    struct Block {
        int* data;
        int units;
        std::map<std::size_t, std::size_t> freeRanges;
    };

    std::vector<Block> blocks;

public:
    Allocator() = default;
    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

    ~Allocator() {
        for (const Block& block : blocks) {
            freeBlock(block.data, block.units);
        }
    }

    int* allocate(int n) {
        const std::size_t count = static_cast<std::size_t>(n);
        // Prefer the newest block, including its unused tail.
        for (auto block = blocks.rbegin(); block != blocks.rend(); ++block) {
            for (auto range = block->freeRanges.begin();
                 range != block->freeRanges.end(); ++range) {
                if (range->second < count) continue;
                const std::size_t offset = range->first;
                const std::size_t remaining = range->second - count;
                if (remaining != 0) {
                    block->freeRanges.emplace(offset + count, remaining);
                }
                block->freeRanges.erase(range);
                return block->data + offset;
            }
        }

        const int units = static_cast<int>((count + intsPerBlock - 1) / intsPerBlock);
        int* data = getNewBlock(units);
        try {
            Block block{data, units, {}};
            const std::size_t capacity = static_cast<std::size_t>(units) * intsPerBlock;
            if (capacity > count) block.freeRanges.emplace(count, capacity - count);
            blocks.push_back(std::move(block));
        } catch (...) {
            freeBlock(data, units);
            throw;
        }
        return data;
    }

    void deallocate(int* pointer, int n) {
        const std::less<int*> before;
        for (Block& block : blocks) {
            int* end = block.data + static_cast<std::size_t>(block.units) * intsPerBlock;
            if (before(pointer, block.data) || !before(pointer, end)) continue;

            const std::size_t offset = static_cast<std::size_t>(pointer - block.data);
            auto range = block.freeRanges.emplace(offset, static_cast<std::size_t>(n)).first;
            // Coalesce both neighbors so an empty block becomes one reusable range.
            if (range != block.freeRanges.begin()) {
                auto previous = range;
                --previous;
                if (previous->first + previous->second == range->first) {
                    previous->second += range->second;
                    block.freeRanges.erase(range);
                    range = previous;
                }
            }
            auto next = range;
            ++next;
            if (next != block.freeRanges.end() &&
                range->first + range->second == next->first) {
                range->second += next->second;
                block.freeRanges.erase(next);
            }
            return;
        }
    }
};

#endif
