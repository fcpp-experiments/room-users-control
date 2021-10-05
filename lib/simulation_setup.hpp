// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file simulation_setup.hpp
 * @brief Setup of the basic simulation details.
 */

#ifndef FCPP_SIMULATION_SETUP_H_
#define FCPP_SIMULATION_SETUP_H_

#include "lib/room_control.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

using namespace component::tags;
using namespace coordination::tags;

//! @brief Sequences and distributions.
//! @{
using spawn_s = sequence::multiple_n<people, 0>;
using log_s = sequence::periodic_n<1, 0, 1, end_time>;
using round_s = sequence::periodic<
    distribution::interval_n<times_t, 0, 1>,
    distribution::weibull_n<times_t, 10, 1, 10>,
    distribution::constant_n<times_t, end_time+2>
>;
using rectangle_d = distribution::rect_n<1, 0, 0, 0, width, height, 0>;
//! @}

//! @brief General options.
DECLARE_OPTIONS(opt,
    tuple_store<
        preference,         real_t,
        satisfaction,       real_t,
        satisfaction_map,   map_type,
        map_size,           size_t,
        local_satmap,       map_type,
        node_state,         automa,
        node_color,         color,
        node_size,          double,
        node_shape,         shape
    >,
    aggregators<
    >,
    parallel<true>,
    synchronised<false>,
    program<coordination::main>,
    exports<coordination::main_t>,
    spawn_schedule<spawn_s>,
    log_schedule<log_s>,
    round_schedule<round_s>,
    init<
        x,              rectangle_d,
        preference,     distribution::interval_n<times_t, 0, 1>
    >,
    dimension<dim>,
    connector<connect::radial<80,connect::fixed<comm,1,dim>>>,
    area<-77, -124, 100*width+77, 100*height+124, 100>,
    size_tag<node_size>,
    shape_tag<node_shape>,
    color_tag<node_color>
);

}


#endif // FCPP_SIMULATION_SETUP_H_
