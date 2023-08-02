#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <xcl2.hpp>
#include "CL/cl_ext_xilinx.h"
#include "graph.h"

#include "helper.h"

#include "util/command_parser.h"

#include "log.h"



int main(int argc, char** argv) {

    log_info("Start");

    InputParser cmd_parser(argc, argv);
    Graph graph;

    if (cmd_parser.check_cmd_option_exists("-vl")) {
        graph.is_vertex_labeled_ = true;
    }

    if (cmd_parser.check_cmd_option_exists("-el")) {
        graph.is_edge_labeled_ = true;
    }

    if (cmd_parser.check_cmd_option_exists("-vw")) {
        graph.is_vertex_weighted_ = true;
    }

    if (cmd_parser.check_cmd_option_exists("-ew")) {
        graph.is_edge_weighted_ = true;
    }

    if (cmd_parser.check_cmd_option_exists("-ep")) {
        graph.is_edge_weight_prefix_summed_ = true;
    }

    if (cmd_parser.check_cmd_option_exists("-al")) {
        graph.is_edge_weight_alias_generated_ = true;
    }

    if (cmd_parser.check_cmd_option_exists("-rj")) {
        graph.is_edge_weight_rejection_generated_ = true;
    }

    graph.is_offset_pair_ = true;

    graph.load_csr(cmd_parser.get_cmd_option("-f"));

    graph.print_metadata();

    return 0;
}
