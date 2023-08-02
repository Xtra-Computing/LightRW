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
    log_info("[qsize] (%d)", qsize);
}

fflush(stdout);

cl_int err;
cl::Context context;
std::vector<cl::Device> devices = xcl::get_xil_devices();
auto fileBuf = xcl::read_binary_file(binaryFile);
cl::Program::Binaries bins{{fileBuf.data(), fileBuf.size()}};
bool valid_device = false;
cl::Device device;
cl::Program cprogram;
for (unsigned int i = 0; i < devices.size(); i++) {
    device = devices[i];
    // Creating Context and Command Queue for selected Device
    OCL_CHECK(err, context = cl::Context(device, nullptr, nullptr, nullptr, &err));

    log_info("Trying to program device[%d]  %s", i , device.getInfo<CL_DEVICE_NAME>().c_str());
    cl::Program program(context, {device}, bins, nullptr, &err);
    if (err != CL_SUCCESS) {
        log_info("Failed to program device[%d] %s", i , device.getInfo<CL_DEVICE_NAME>().c_str());
    } else {
        log_info("Device[%d]: program successful!", i);
        valid_device = true;
        cprogram =  program;
        break; // we break because we found a valid device
    }
}
if (!valid_device) {
    log_info("Failed to program any device found, exit!");
    exit(EXIT_FAILURE);
}

pcg32_srandom(42u, 54u);
uint32_t vertex_num = graph.num_vertices();


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
    if (query_vertex > vertex_num)
    {
        break;
    }
}
shuffle(query_array.data(), query_array.size());
uint32_t query_num;

if (qsize  == 0)
{
    query_num = get_aligned_size(query_array.size(), 256 * ACC_NUM);
}
else
{
    query_num = (qsize < 512 * ACC_NUM) ? 512 * ACC_NUM : qsize ;
}

DEBUG_PRINTF("query num %d (%ld)(%d) ", query_num, query_array.size(), qsize);
fflush(stdout);

vd_mem_t vertex_mem(vertex_num);
for (size_t i = 0; i < vertex_num; i++) {
    vertex_descriptor_t v;
    v.start = graph.offset_[i];
    v.size  = graph.degree(i);
    vertex_mem[i] = v;
}

uint32_t dest_vertex_num = graph.num_edges();
DEBUG_PRINTF("edge num %d ", dest_vertex_num);
dest_mem_t dest_vertex_mem(dest_vertex_num);
weight_mem_t weight(dest_vertex_num);
for (size_t i = 0; i < dest_vertex_num; i++) {
    dest_vertex_mem[i] = graph.adj_[i];  //edge
    if (vertex_mem[dest_vertex_mem[i]].size <= 1)
    {
        //dest_vertex_mem[i] = dest_vertex_mem[i] | 0x80000000;
    }
    weight[i] = (pcg32_random() % max_weight_value) + 1; //edge prop
}
