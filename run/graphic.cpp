// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "lib/simulation_setup.hpp"

using namespace fcpp;

int main() {
    using net_t = typename component::interactive_simulator<opt>::net;
    net_t network{common::make_tagged_tuple<name,epsilon>("Room Users' Control", 0.1)};
    network.run();
    return 0;
}
