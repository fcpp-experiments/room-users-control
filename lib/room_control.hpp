// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file room_control.hpp
 * @brief Users' control and satisfaction in a room.
 */

#ifndef FCPP_ROOM_CONTROL_H_
#define FCPP_ROOM_CONTROL_H_

#include <array>
#include <functional>

#include "lib/fcpp.hpp"


//! @brief Number of people in the room.
constexpr size_t people = 80;

//! @brief Communication radius.
constexpr size_t comm = 25;

//! @brief Width of the room.
constexpr size_t width = 100;

//! @brief Height of the room.
constexpr size_t height = 30;

//! @brief Tallness of people.
constexpr size_t tall = 5;

//! @brief Granularity of the map reconstruction.
constexpr size_t grain = 10;

//! @brief X size of the reconstructed map.
constexpr size_t map_x_size = (width + grain - 1) / grain;

//! @brief Y size of the reconstructed map.
constexpr size_t map_y_size = (height + grain - 1) / grain;

//! @brief Dimensionality of the space.
constexpr size_t dim = 3;

//! @brief The end of simulated time.
constexpr size_t end_time = 1000;

//! @brief The type of indices in the satisfaction map.
using index_type = std::array<size_t, 2>;

//! @brief Hasher for indices.
namespace std {
    template <>
    struct hash<index_type> {
        size_t operator()(index_type const& i) const {
            return i[0] + i[1]*map_x_size;
        }
    };
}


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


namespace tags {
    //! @brief Heat preference of a user.
    struct preference {};

    //! @brief Satisfaction of the node.
    struct satisfaction {};

    //! @brief Map of satisfaction of nodes.
    struct satisfaction_map {};

    //! @brief Size of the current node.
    struct node_size {};

    //! @brief Shape of the current node.
    struct node_shape {};

    //! @brief Color representing the node.
    struct node_color {};
}

//! @brief Main function.
MAIN() {
    using namespace tags;

    real_t s = node.position()[1]/height - node.storage(preference{});
    node.storage(satisfaction{}) = s;
    if (s > 0)
        node.storage(node_color{}) = s * color(CRIMSON) + (1-s) * color(SILVER);
    else {
        s = -s;
        node.storage(node_color{}) = s * color(DEEP_SKY_BLUE) + (1-s) * color(SILVER);
    }
    node.storage(node_size{}) = 1.5;
    node.storage(node_shape{}) = shape::sphere;
    rectangle_walk(CALL, make_vec(0,0,tall), make_vec(width,height,tall), 10, 1);
}
FUN_EXPORT main_t = common::export_list<rectangle_walk_t<dim>>;


}


}

#endif // FCPP_ROOM_CONTROL_H_
