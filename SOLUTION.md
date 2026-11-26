# Allocator implementation

`src.hpp` implements the required integer allocator. Each acquired block owns
an ordered map of free ranges, measured in integers. Allocation searches the
newest block first, splits a suitable range, and acquires the minimum number of
4096-byte units only when no existing range fits. Deallocation merges adjacent
free ranges. Empty blocks remain available for reuse, and destruction releases
every acquired block with its original unit count.

With A allocation calls, the number of blocks and free ranges is O(A).
Allocation and deallocation take O(A) time in the worst case, with O(A)
bookkeeping space. Returned storage comes exclusively from getNewBlock.

## Local validation

The advertised sample directory was absent. The local test driver covers packed
single-integer allocations, reuse of the most recent allocation, coalescing an
empty block, randomized allocation/deallocation with data integrity checks, and
cleanup of allocations that callers did not release.

Command (passed):

```sh
g++ -std=c++11 -Wall -Wextra -Wpedantic -O1 -g -fsanitize=address,undefined tests/allocator_test.cpp -o /tmp/problem065_test
/tmp/problem065_test
```

The supplied submission client lacked the `submit_code` method used by its CLI.
That method was added using the existing API request and submission logging
helpers.

## Online judge result

- Submission ID: 879513
- Submitted implementation commit: 3e7d123
- Result: Accepted, 100/100
- All 20 test points passed, including 10 memory-check tests.
- Submission attempts used: 1 of 5.
