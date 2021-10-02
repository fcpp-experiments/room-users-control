// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file room_control.hpp
 * @brief Users' control and satisfaction in a room.
 */

#ifndef FCPP_ROOM_CONTROL_H_
#define FCPP_ROOM_CONTROL_H_

#include <cassert>
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


enum class automa {
    sitting, walking, reporting, following, gateway
};

std::string to_string(automa a) {
    switch (a) {
        case automa::sitting:
            return "sitting";

        case automa::walking:
            return "walking";

        case automa::reporting:
            return "reporting";

        case automa::following:
            return "following";

        case automa::gateway:
            return "gateway";

        default:
            return "automa";
    }
}
constexpr std::array<size_t, 5> automa_tall = {3, 5, 3, 5, 10};

constexpr std::array<real_t, 5> automa_size = {1.5, 1.4, 1.2, 1.2, 2};

constexpr std::array<shape, 5> automa_shape = {shape::cube, shape::sphere, shape::star, shape::star, shape::tetrahedron};

constexpr std::array<std::array<size_t, 5>, 5> automa_transition{{
    {95,  1,  4,  0,  0},
    {10, 90,  0,  0,  0},
    {30,  0, 60, 10,  0},
    {20,  0,  0, 80,  0},
    { 0,  0,  0,  0,100}
}};


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


namespace tags {
    //! @brief Heat preference of a user.
    struct preference {};

    //! @brief Satisfaction of the node.
    struct satisfaction {};

    //! @brief Map of satisfaction of nodes.
    struct satisfaction_map {};

    //! @brief Automa state of the current node.
    struct node_state {};

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
    if (node.uid == 0) node.storage(node_color{}) = color(YELLOW);
    automa a = old(CALL, node.uid == 0 ? automa::gateway : automa::sitting, [&](automa a){
        int r = node.next_int(99);
        for (size_t i=0; i<5; ++i) {
            r -= automa_transition[(size_t)a][i];
            if (r < 0) return (automa)i;
        }
        assert(false);
        return automa::sitting;
    });
    node.storage(node_state{}) = a;
    node.storage(node_size{}) = automa_size[(size_t)a];
    node.storage(node_shape{}) = automa_shape[(size_t)a];
    real_t t = automa_tall[(size_t)a];
    switch (a) {
        case automa::gateway:
        {
            node.position() = make_vec(width/2,height/2,t);
            break;
        }
        case automa::sitting:
        case automa::reporting:
        {
            vec<3> p = node.position();
            p[2] = t;
            node.position() = p;
            node.velocity() = make_vec(0,0,0);
            break;
        }
        case automa::walking:
        {
            rectangle_walk(CALL, make_vec(0,0,t), make_vec(width,height,t), 5, 1);
            break;
        }
        case automa::following:
        {
            real_t h = node.storage(preference{})*height;
            rectangle_walk(CALL, make_vec(0,h,t), make_vec(width,h,t), 5, 1);
            break;
        }
    }
    std::unordered_map<index_type, real_t> sm;
    if (a == automa::reporting) {
        index_type idx;
        for (int i=0; i<2; ++i)
            idx[i] = node.position()[i]/grain;
        sm.emplace(idx, s);
    }
    node.storage(satisfaction_map{}) = sm;
}
FUN_EXPORT main_t = common::export_list<rectangle_walk_t<dim>, automa>;


}


}

#endif // FCPP_ROOM_CONTROL_H_
