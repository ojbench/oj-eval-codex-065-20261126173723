#include "../src.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>

static std::map<const int*, int> owned;
static int requests = 0;

int* getNewBlock(int n) {
    assert(n > 0);
    ++requests;
    int* p = new int[static_cast<std::size_t>(n) * 4096 / sizeof(int)];
    owned.emplace(p, n);
    return p;
}

void freeBlock(const int* p, int n) {
    assert(owned.count(p) && owned.at(p) == n);
    owned.erase(p);
    delete[] p;
}

int main() {
    {
        Allocator a;
        assert(requests == 0);
        for (int i = 0; i < 5000; ++i) *a.allocate(1) = i;
        assert(requests == 5);
    }
    assert(owned.empty());
    {
        Allocator a;
        int* p = a.allocate(100);
        int* q = a.allocate(2000);
        a.deallocate(q, 2000);
        const int oldRequests = requests;
        assert(a.allocate(2000) == q);
        assert(requests == oldRequests);
        a.deallocate(p, 100);
    }
    {
        Allocator a;
        int* p = a.allocate(100);
        int* q = a.allocate(200);
        int* r = a.allocate(724);
        a.deallocate(q, 200);
        a.deallocate(p, 100);
        a.deallocate(r, 724);
        const int oldRequests = requests;
        assert(a.allocate(1024) == p);
        assert(requests == oldRequests);
    }
    std::mt19937 random(1766);
    struct Allocation { int* p; int n; int value; };
    for (int trial = 0; trial < 100; ++trial) {
        Allocator a;
        std::vector<Allocation> live;
        for (int step = 0; step < 999; ++step) {
            if (!live.empty() && random() % 2 == 0) {
                std::size_t i = random() % live.size();
                a.deallocate(live[i].p, live[i].n);
                live.erase(live.begin() + i);
            } else {
                int n = 1 + random() % 5000;
                int* p = a.allocate(n);
                std::fill(p, p + n, step);
                live.push_back({p, n, step});
            }
            for (const auto& entry : live) {
                assert(std::all_of(entry.p, entry.p + entry.n,
                    [&](int value) { return value == entry.value; }));
            }
        }
    }
    assert(owned.empty());
    std::cout << "All allocator tests passed\n";
}
