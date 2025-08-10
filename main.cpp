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

    auto fromDoubleI = (absl::int128)9223372036854775808.0;
    std::cout << fromDoubleI << std::endl;
    auto fromDoubleU = (absl::uint128)18446744073709551616.0;
    std::cout << fromDoubleU << std::endl;
    return 0;
}
