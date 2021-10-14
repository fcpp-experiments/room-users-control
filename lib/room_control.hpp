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
constexpr size_t people = 82;

//! @brief Communication radius.
constexpr size_t comm = 4;

//! @brief Width of the room.
constexpr size_t width = 18;

//! @brief Height of the room.
constexpr size_t height = 9;

//! @brief Tallness of people in cm.
constexpr size_t tall = 170;

//! @brief Granularity of the map reconstruction.
constexpr size_t grain = 3;

//! @brief X size of the reconstructed map.
constexpr size_t map_x_size = (width + grain - 1) / grain;

//! @brief Y size of the reconstructed map.
constexpr size_t map_y_size = (height + grain - 1) / grain;

//! @brief Dimensionality of the space.
constexpr size_t dim = 3;

//! @brief The end of simulated time.
constexpr size_t end_time = 1000;

//! @brief Time after which a preference is removed.
constexpr size_t decay_time = 30;

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

//! @brief The type of the satisfaction map.
using map_type = std::unordered_map<index_type, real_t>;


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
constexpr std::array<real_t, 5> automa_tall = {0.6, 1, 0.6, 1, 2};

constexpr std::array<real_t, 5> automa_size = {.3, .28, .24, .24, .4};

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

    //! @brief Local map of nodes satisfaction.
    struct local_satmap {};

    //! @brief Map of satisfaction of nodes.
    struct satisfaction_map {};

    //! @brief Size of the satisfaction map.
    struct map_size {};

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

    // compute and display satisfaction
    real_t s = node.position()[1]/height - node.storage(preference{});
    node.storage(satisfaction{}) = s;
    if (s > 0)
        node.storage(node_color{}) = s * color(CRIMSON) + (1-s) * color(SILVER);
    else
        node.storage(node_color{}) = -s * color(DEEP_SKY_BLUE) + (1+s) * color(SILVER);
    if (node.uid == 0) node.storage(node_color{}) = color(YELLOW);

    // update and display automa state
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
    node.storage(node_size{}) = automa_size[(size_t)a] * tall / 100;
    node.storage(node_shape{}) = automa_shape[(size_t)a];

    // handle movement according to automa state
    real_t t = automa_tall[(size_t)a] * tall / 100;
    switch (a) {
        case automa::gateway:
        {
            node.position() = make_vec(width*0.5,height*0.5,t);
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
            if (node.position()[2] == 0) {
                vec<3> p = node.position();
                p[2] = t;
                node.position() = p;
            }
            rectangle_walk(CALL, make_vec(0,0,t), make_vec(width,height,t), grain, 1.4, 1);
            break;
        }
        case automa::following:
        {
            real_t h = node.storage(preference{})*height;
            real_t w1 = node.position()[0];
            real_t w2 = node.position()[0];
            w1 = max(w1 - grain, real_t(0));
            w2 = min(w2 + grain, real_t(width));
            if (follow_target(CALL, constant(CALL, random_rectangle_target(CALL, make_vec(w1,h,t), make_vec(w2,h,t))), 1.4, 1) < 0.01)
                a = automa::sitting;
            break;
        }
    }

    // initialise satisfaction map with reported satisfacion
    map_type sm;
    if (a == automa::reporting) {
        index_type idx;
        for (int i=0; i<2; ++i)
            idx[i] = node.position()[i]/grain;
        sm.emplace(idx, s);
    }
    // persist map entry for given decay time
    sm = timed_decay(CALL, sm, map_type{}, decay_time);
    // average map entry with neighbours
    field<map_type> nsm = nbr(CALL, sm);
    if (not sm.empty()) {
        assert(sm.size() == 1);
        using tuple_type = tuple<real_t, size_t>;
        field<tuple_type> same_cell_pref = map_hood([&](map_type const& m) {
            return m.size() == 1 and m.begin()->first == sm.begin()->first ? tuple_type(m.begin()->second, 1) : tuple_type(0, 0);
        }, nsm);
        tuple_type tot_pref = sum_hood(CALL, same_cell_pref, tuple_type(sm.begin()->second, 1));
        sm.begin()->second = get<0>(tot_pref) / get<1>(tot_pref);
    }
    // store map entry
    node.storage(local_satmap{}) = sm;
    // collect satisfaction towards the gateway
    hops_t dist = abf_hops(CALL, a == automa::gateway);
    sm = mp_collection(CALL, dist, sm, map_type{}, [](map_type m1, map_type m2){
        for (auto const& x : m2)
            m1[x.first] = x.second;
        return m1;
    }, [](map_type m, size_t){
        return m;
    });
    node.storage(satisfaction_map{}) = sm;
    node.storage(map_size{}) = sm.size();
    if (a == automa::gateway) {
        std::cerr << std::endl << "T = " << node.current_time() << std::endl;
        index_type idx;
        for (idx[1]=0; idx[1]<3; ++idx[1]) {
            for (idx[0]=0; idx[0]<6; ++idx[0]) {
                if (sm.count(idx))
                    std::cerr << sm.at(idx) << "\t";
                else
                    std::cerr << "*\t";
            }
            std::cerr << std::endl;
        }
    }
}
FUN_EXPORT main_t = common::export_list<rectangle_walk_t<dim>, constant_t<vec<dim>>, timed_decay_t<map_type>, abf_hops_t, mp_collection_t<hops_t, map_type>, map_type, automa>;


}


}

#endif // FCPP_ROOM_CONTROL_H_
