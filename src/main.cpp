#include <cstddef>
#include <cstdlib>
#include <string>
#include <iostream>
#include <list>
#include <algorithm>

#include "TreeMap.h"
#include "HashMap.h"

namespace
{

    template <typename K, typename V>
    using Map = aisdi::TreeMap<K, V>;

    void perfomTest()
    {
        Map<int, std::string> map = { { 753, "Rome" } };
        map.remove(753);
    }

} // namespace

int main(int argc, char** argv)
{
    const std::size_t repeatCount = argc > 1 ? std::atoll(argv[1]) : 1;
    for (std::size_t i = 0; i < repeatCount; ++i)
        perfomTest();
    return 0;
}
