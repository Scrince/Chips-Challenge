#include "viewport.hpp"

#include <cstdlib>
#include <iostream>

namespace {
int failures{};
void check(bool condition, const char* name) {
    if (!condition) {
        std::cerr << "FAIL: " << name << '\n';
        ++failures;
    }
}
}

int main() {
    using chips_native::playerViewport;
    const auto center = playerViewport(16, 16, 32, 32);
    check(center.left == 12 && center.top == 12, "center camera follows player");
    check(center.width == 9 && center.height == 9, "viewport remains exactly 9x9");
    check(16 - center.left == 4 && 16 - center.top == 4, "centered player occupies center cell");

    const auto northwest = playerViewport(0, 0, 32, 32);
    check(northwest.left == 0 && northwest.top == 0, "northwest edge clamps camera");
    check(0 - northwest.left == 0 && 0 - northwest.top == 0, "edge player remains visible");

    const auto southeast = playerViewport(31, 31, 32, 32);
    check(southeast.left == 23 && southeast.top == 23, "southeast edge clamps camera");
    check(31 - southeast.left == 8 && 31 - southeast.top == 8, "edge player remains in final viewport cell");

    const auto east = playerViewport(31, 16, 32, 32);
    const auto south = playerViewport(16, 31, 32, 32);
    check(east.left == 23 && east.top == 12, "east edge keeps vertical follow");
    check(south.left == 12 && south.top == 23, "south edge keeps horizontal follow");
    check(playerViewport(4, 4, 8, 8).width == 0, "undersized map rejected");

    if (failures == 0) std::cout << "All viewport checks passed.\n";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
