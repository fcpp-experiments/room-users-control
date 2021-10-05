// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "lib/simulation_setup.hpp"

using namespace fcpp;

int main() {
    using net_t = typename component::interactive_simulator<opt>::net;
    net_t network{common::make_tagged_tuple<name,epsilon,texture>("Room Users' Control", 0.1, "building.jpg")};
    network.run();
    return 0;
}
