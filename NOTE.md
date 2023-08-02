#connect info
slr=vmult_1:SLR1
slr=vadd_1:SLR1
stream_connect=krnl_stream_vadd_1.out:krnl_stream_vmult_1.in2:64



#debug
add_wave {{/pfm_top_wrapper/pfm_top_i/pfm_dynamic_inst/slr1/interconnect_axilite_user_1/M00_AXI}} 
add_wave {{/pfm_top_wrapper/pfm_top_i/pfm_dynamic_inst/slr1/interconnect_axilite_user_1/M01_AXI}} 
add_wave {{/pfm_top_wrapper/pfm_top_i/pfm_dynamic_inst/slr1/interconnect_axilite_user_1/M02_AXI}} 
add_wave {{/pfm_top_wrapper/pfm_top_i/pfm_dynamic_inst/slr1/interconnect_axilite_user_1/M03_AXI}} 
...


Read Data --> RDATA --> Go Source --> ".M05_AXI_rdata(slr1_M05_AXI_RDATA)" -->   "pfm_dynamic_staburst_core_1_0 staburst_core_1"