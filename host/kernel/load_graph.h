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


if (cmd_parser.check_cmd_option_exists("-graph"))
{
    log_info("%s", cmd_parser.get_cmd_option("-graph").c_str());
    graph.load_csr(cmd_parser.get_cmd_option("-graph"));
} else
{
    graph.load_csr("/data/graph_at_scale/youtube");
    log_info("default graph");
}


graph.print_metadata();
std::string binaryFile;

if (cmd_parser.check_cmd_option_exists("-fpga"))
    binaryFile = cmd_parser.get_cmd_option("-fpga");
else
{
    std::string app_name = argv[0];
    app_name = app_name.substr(2, app_name.size() - 6);
    binaryFile = "build_dir_" + app_name + "/kernel.xclbin";
}
log_info("FPGA bitstream: %s", binaryFile.c_str());

int qsize = 0;
if (cmd_parser.check_cmd_option_exists("-qsize")) {
    qsize = std::stoi(cmd_parser.get_cmd_option("-qsize"));
    log_info("[qsize] (%d) \n", qsize);
}

fflush(stdout);


std::vector<uint32_t> query_array;

uint32_t query_vertex = 0;
while (1)
{
    auto degree = graph.degree(query_vertex);
    if (degree != 0)
    {
        query_array.push_back(query_vertex);
    }
    query_vertex += 1;
    if (query_vertex > (uint32_t) (graph.num_vertices()))
    {
        break;
    }
}
shuffle(query_array.data(), query_array.size());
uint32_t query_num;

if (qsize  == 0)
{
    query_num = get_aligned_size(query_array.size(), 256);
}
else
{
    query_num = (qsize < 512 ) ? 512  : qsize ;
}

DEBUG_PRINTF("query num %d (%ld)(%d) \n", query_num, query_array.size(), qsize);
fflush(stdout);
