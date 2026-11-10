#include "input_mapping.hpp"

#include <array>
#include <iostream>
#include <span>

int main() {
    int failures = 0;
    const std::array<std::uint16_t, 4> bindings{26, 7, 22, 4};
    const std::array<std::uint16_t, 4> arrows{82, 79, 81, 80};
    const auto check = [&](bool condition, const char* description) {
        if (!condition) {
            std::cerr << "FAIL: " << description << '\n';
            ++failures;
        }
    };
    for (std::uint16_t i = 0; i < bindings.size(); ++i)
        check(chips::movementIndexForKey(bindings[i], bindings, arrows) == i,
              "configured movement key maps to its direction index");
    for (std::uint16_t i = 0; i < arrows.size(); ++i)
        check(chips::movementIndexForKey(arrows[i], bindings, arrows) == i,
              "fixed alternate movement key maps to its direction index");
    check(chips::movementIndexForKey(99, bindings, arrows) == -1,
          "unmapped key has no movement action");
    const std::array<std::uint16_t, 3> reserved{41, 40, 58};
    check(chips::isReservedKey(40, reserved), "reserved application key is rejected");
    check(!chips::isReservedKey(99, reserved), "unreserved key remains available");
    check(chips::validateMovementBinding(99, 0, bindings, reserved, arrows) ==
              chips::BindingRejection::none,
          "new unassigned key can be bound");
    check(chips::validateMovementBinding(bindings[1], 0, bindings, reserved, arrows) ==
              chips::BindingRejection::duplicate,
          "key assigned to another direction is rejected");
    check(chips::validateMovementBinding(arrows[0], 0, bindings, reserved, arrows) ==
              chips::BindingRejection::reserved,
          "fixed arrow alternate cannot be overwritten");
    check(chips::validateMovementBinding(40, 0, bindings, reserved, arrows) ==
              chips::BindingRejection::reserved,
          "reserved application control cannot be overwritten");
    return failures == 0 ? 0 : 1;
}
