#include "int128.h"
#include <cstdint>
#include <iostream>
#include <limits>

int main()
{
    absl::int128 x = 1;
    x += std::numeric_limits<int64_t>::max();
    std::cout << x << std::endl;
    absl::uint128 y = 1;
    y += std::numeric_limits<uint64_t>::max();
    std::cout << y << std::endl;
    return 0;
}
