//
// Copyright 2021 Xilinx, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_uuid.h"

#include "utils_XAcc_jpeg.hpp"

#include "common.h"

// Kernel arguments index definition
#define krnl_jpeg_jpeg_ptr          0
#define krnl_jpeg_jpeg_size         1
#define krnl_jpeg_yuv_ptr           2
#define krnl_jpeg_infos_ptr         3

#define krnl_yuv_mover_ctrl_size    0
#define krnl_yuv_mover_data_size    1
#define krnl_yuv_mover_ctrl_ptr     2
#define krnl_yuv_mover_data_ptr     3

#define krnl_rgb_mover_img_width    0
#define krnl_rgb_mover_img_height   1
#define krnl_rgb_mover_buffer_ptr   2

#define DEVICE_ID   0

// BMP file head structure
uint8_t BmpFileHead[54] = {
    0x42, 0x4d,             // bfType:              byte  0 -  1
    0x00, 0x00, 0x00, 0x00, // bfSize:              byte  2 -  5
    0x00, 0x00,             // bfReserved1:         byte  6 -  7
    0x00, 0x00,             // bfReserved2:         byte  8 -  9
    0x36, 0x00, 0x00, 0x00, // bfOffBits:           byte 10 - 13
    0x28, 0x00, 0x00, 0x00, // biSize:              byte 14 - 17
    0x00, 0x00, 0x00, 0x00, // biWidth:             byte 18 - 21
    0x00, 0x00, 0x00, 0x00, // biHeight:            byte 22 - 25
    0x01, 0x00,             // biPlanes:            byte 26 - 27
    0x20, 0x00,             // biBitCount:          byte 28 - 29
    0x00, 0x00, 0x00, 0x00, // biCompression:       byte 30 - 33
    0x00, 0x00, 0x00, 0x00, // biSizeImages:        byte 34 - 37
    0x00, 0x00, 0x00, 0x00, // biXPelsPerMeter:     byte 38 - 41
    0x00, 0x00, 0x00, 0x00, // biYPelsPerMeter:     byte 42 - 46
    0x00, 0x00, 0x00, 0x00, // biClrUsed:           byte 47 - 50
    0x00, 0x00, 0x00, 0x00  // biClrImportant:      byte 51 - 54
};

// print command line help
void print_help(void) {
    std::cout << std::endl << "    Usage: host_everlay.exe -i JPEGFILE [-c COLOR]" << std::endl << std::endl;
}

// pixel value saturation, int32 -> uint8
uint8_t pixel_sat_32_8(int32_t in)
{
	uint8_t result;
	if (in < 0) {
		result = 0;
	} else if (in > 255) {
		result = 255;
	} else {
		result = in;
	}
	return result;
}

// write RGB444 data to BMP file
void write_bmp(uint8_t *rgb_array, std::string file_name, int image_width, int image_height)
{
    int file_size = image_height * image_width * 4 + 54; 

    FILE *fp;

    uint32_t temp;

    // complement BMP file head array
    temp = file_size;
    for (int i = 2; i < 6; i++)
    {
        BmpFileHead[i] = temp & 0xff;
        temp = temp >> 8;
    }

    temp = image_width;
    for (int i = 18; i < 22; i++)
    {
        BmpFileHead[i] = temp & 0xff;
        temp = temp >> 8;
    }    

    temp = image_height;
    for (int i = 22; i < 26; i++)
    {
        BmpFileHead[i] = temp & 0xff;
        temp = temp >> 8;
    }    
    
    fp = fopen(file_name.c_str(), "wb");

    // write file header
    fwrite(BmpFileHead, 1, 54, fp);
    
    // write pixel date
    for (int i = (image_height - 1); i >= 0; i--)
    {
        fwrite((rgb_array + 4 * image_width * i), 1, (image_width * 4), fp);
    }
    
    fclose(fp);

}

// reorder JPEG decoded YUV pixel, finish color space conversion and write out reference BMP file
void rebuild_image( xf::codec::bas_info* bas_info,
                     uint8_t* yuv_mcu_pointer) 
{   

    int image_width = bas_info->axi_width[0] * 8;
    int image_height = bas_info->axi_height[0] * 8;  

    // reorg yuv data to three planes 
   uint8_t* yuv_plane[3];
   for (int i = 0; i < 3; i++) {
       yuv_plane[i] = (uint8_t*)malloc(image_width * image_height);
   }

   for (int mcuv_cnt = 0; mcuv_cnt < bas_info->axi_height[0]; mcuv_cnt++)
   {
       for (int mcuh_cnt = 0; mcuh_cnt < bas_info->axi_width[0]; mcuh_cnt++)
       {
           for (int i = 0; i < 3; i++) // three components (Y,U,V)
           { 
               for (int col = 0; col < 8; col++)
               {
                   for (int row = 0; row < 8; row++) {
                       uint64_t offset = (8 * mcuv_cnt + row) * bas_info->axi_width[0] * 8 + 8 * mcuh_cnt + col;
                       yuv_plane[i][offset] = *yuv_mcu_pointer;
                       yuv_mcu_pointer++;
                   }
               }
           }
       }
   }    
   
   // color space convertion
   uint8_t*  rgb_plane;
   rgb_plane = (uint8_t*)malloc(image_width * image_height * 4);   // RGBA format
   
   for (int row = 0; row < image_height; row++)
   {
       for (int col = 0; col < image_width; col++)
       {
            int16_t y = (uint16_t)yuv_plane[0][image_width * row + col] - 16;
            int16_t u = (uint16_t)yuv_plane[1][image_width * row + col] - 128;
            int16_t v = (uint16_t)yuv_plane[2][image_width * row + col] - 128;

            int32_t r = (76284 * y + 104595 * v) >> 16;
            int32_t g = (76284 * y -  53281 * v - 25625 * u) >> 16;
            int32_t b = (76284 * y + 132252 * u) >> 16;

            rgb_plane[(image_width * row + col) * 4 + 0] = pixel_sat_32_8(b);
            rgb_plane[(image_width * row + col) * 4 + 1] = pixel_sat_32_8(g);
            rgb_plane[(image_width * row + col) * 4 + 2] = pixel_sat_32_8(r);
            rgb_plane[(image_width * row + col) * 4 + 3] = 0;
       }
   }
    
    // write pixel data
    write_bmp(rgb_plane, "./decoded.bmp", image_width, image_height);

   std::cout << "The JPEG file is decoded to image 'decoded.bmp' of " << image_width << " by " << image_height << std::endl;

}

// extract image information from output of JPEG decoder kernel
void rebuild_infos(xf::codec::img_info& img_info,
                   xf::codec::cmp_info cmp_info[MAX_NUM_COLOR],
                   xf::codec::bas_info& bas_info,
                   int& rtn,
                   int& rtn2,
                   uint32_t infos[1024]) {

    img_info.hls_cs_cmpc = *(infos + 0);
    img_info.hls_mcuc = *(infos + 1);
    img_info.hls_mcuh = *(infos + 2);
    img_info.hls_mcuv = *(infos + 3);

    rtn = *(infos + 4);
    rtn2 = *(infos + 5);


    bas_info.all_blocks = *(infos + 10);

    for (int i = 0; i < MAX_NUM_COLOR; i++) {
        bas_info.axi_height[i] = *(infos + 11 + i);
    }

    for (int i = 0; i < 4; i++) {
        bas_info.axi_map_row2cmp[i] = *(infos + 14 + i);
    }

    bas_info.axi_mcuv = *(infos + 18);
    bas_info.axi_num_cmp = *(infos + 19);
    bas_info.axi_num_cmp_mcu = *(infos + 20);

    for (int i = 0; i < MAX_NUM_COLOR; i++) {
        bas_info.axi_width[i] = *(infos + 21 + i);
    }

    int format = *(infos + 24);
    bas_info.format = (xf::codec::COLOR_FORMAT)format;

    for (int i = 0; i < MAX_NUM_COLOR; i++) {
        bas_info.hls_mbs[i] = *(infos + 25 + i);
    }

    bas_info.hls_mcuc = *(infos + 28);

    for (int c = 0; c < MAX_NUM_COLOR; c++) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                bas_info.idct_q_table_x[c][i][j] = *(infos + 29 + c * 64 + i * 8 + j);
            }
        }
    }
    for (int c = 0; c < MAX_NUM_COLOR; c++) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                bas_info.idct_q_table_y[c][i][j] = *(infos + 221 + c * 64 + i * 8 + j);
            }
        }
    }
    
    bas_info.mcu_cmp = *(infos + 413);

    for (int c = 0; c < MAX_NUM_COLOR; c++) {
        for (int i = 0; i < 64; i++) {
            bas_info.min_nois_thld_x[c][i] = *(infos + 414 + c * 64 + i);
        }
    }
    for (int c = 0; c < MAX_NUM_COLOR; c++) {
        for (int i = 0; i < 64; i++) {
            bas_info.min_nois_thld_y[c][i] = *(infos + 606 + c * 64 + i);
        }
    }
    for (int c = 0; c < MAX_NUM_COLOR; c++) {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                bas_info.q_tables[c][i][j] = *(infos + 798 + c * 64 + i * 8 + j);
            }
        }
    }

    for (int c = 0; c < MAX_NUM_COLOR; c++) {
        cmp_info[c].bc = *(infos + 990 + c * 6);
        cmp_info[c].bch = *(infos + 991 + c * 6);
        cmp_info[c].bcv = *(infos + 992 + c * 6);
        cmp_info[c].mbs = *(infos + 993 + c * 6);
        cmp_info[c].sfh = *(infos + 994 + c * 6);
        cmp_info[c].sfv = *(infos + 995 + c * 6);
    }

}

// function to read JPEG image as binary file
void read_file(const char *file_name, int file_size, char *read_buffer)
{
    // read jpeg image as binary file to host memory
    std::ifstream input_file(file_name, std::ios::in | std::ios::binary);
    input_file.read(read_buffer, file_size);
    input_file.close();
}

// Main program body
int main(int argc, char *argv[]) {

    int file_size;      // input JPEG file size (bytes)
    int overlay_color_red   = 0x60;
    int overlay_color_green = 0x60;
    int overlay_color_blue  = 0xff;
    
    int opt;
    const char *optstring = "i:c:";
    std::string file_name;
    int color_bpp24;


    if (argc == 1) 
    {
         print_help();
         return EXIT_SUCCESS;
    } 

    while ((opt = getopt(argc, argv, optstring)) != -1) {
        if ((opt == 'i') && optarg) {
            file_name = std::string(optarg);
        }
        if ((opt == 'c') && optarg) {
            color_bpp24 = std::stoi(std::string(optarg), nullptr, 16);
            overlay_color_red   = (color_bpp24 >> 16) & 0xff;
            overlay_color_green = (color_bpp24 >> 8) & 0xff;
            overlay_color_blue  =  color_bpp24 & 0xff;
        }
    }

// --------------------------------------------------------------------------------------
// check input JPEG file size
// --------------------------------------------------------------------------------------   
    struct stat statbuff;
    if (stat(file_name.c_str(), &statbuff)) {
        std::cout << "Cannot open file " << file_name << std::endl;
        return EXIT_FAILURE;
    }
    file_size = statbuff.st_size;
    std::cout << "Input JPEG file size = " << file_size << std::endl;

    
// --------------------------------------------------------------------------------------
// Device and kernel initialization
// --------------------------------------------------------------------------------------
    // Judge emulation mode accoring to env variable
    std::string xclbin_file;
    char *env_emu;
    if (env_emu = getenv("XCL_EMULATION_MODE")) {
        std::string mode(env_emu);
        if (mode == "hw_emu")
        {
            std::cout << "Program running in hardware emulation mode" << std::endl;
            xclbin_file = "overlay_hw_emu.xclbin";
        }
        else
        {
            std::cout << "[ERROR] Unsupported Emulation Mode: " << mode << std::endl;
            return EXIT_FAILURE;
        }
    }
    else {
        std::cout << "Program running in hardware mode" << std::endl;
        xclbin_file = "build_dir_aie_test/kernel.xclbin";
    }

    // Load xclbin
    std::cout << "Load " << xclbin_file << std::endl;
    xrt::device device = xrt::device(DEVICE_ID);
    xrt::uuid xclbin_uuid = device.load_xclbin(xclbin_file);
   
    // create kernel objects
    std::cout << "Create kernels" << std::endl;
    xrt::kernel krnl_jpeg      = xrt::kernel(device, xclbin_uuid, "krnl_jpeg");
    xrt::kernel krnl_yuv_mover = xrt::kernel(device, xclbin_uuid, "krnl_yuv_mover");
    xrt::kernel krnl_rgb_mover = xrt::kernel(device, xclbin_uuid, "krnl_rgb_mover");


// --------------------------------------------------------------------------------------
// JPEG decoding
// --------------------------------------------------------------------------------------
    // create host buffer for host-device data exchange
    uint8_t *jpeg_data;    // host buffer for input JPEG file
    uint8_t *yuv_data;     // host buffer for decoded YUV planner image data
    uint8_t *infos_data;   // host buffer for JPEG file information packet

    jpeg_data  = new uint8_t [file_size];
    yuv_data   = new uint8_t [MAXCMP_BC*64];
    infos_data = new uint8_t [4096];

    // get memory bank groups for device buffer
    xrtMemoryGroup bank_grp_jpeg  = krnl_jpeg.group_id (krnl_jpeg_jpeg_ptr);
    xrtMemoryGroup bank_grp_yuv   = krnl_jpeg.group_id (krnl_jpeg_yuv_ptr);
    xrtMemoryGroup bank_grp_infos = krnl_jpeg.group_id (krnl_jpeg_infos_ptr);

    // create device buffer objects
    std::cout << "Create input and output device buffers for JPEG decoder" << std::endl;
    xrt::bo jpeg_buffer  = xrt::bo (device, file_size,    xrt::bo::flags::normal, bank_grp_jpeg);
    xrt::bo yuv_buffer   = xrt::bo (device, MAXCMP_BC*64, xrt::bo::flags::normal, bank_grp_yuv);
    xrt::bo infos_buffer = xrt::bo (device, 4096,         xrt::bo::flags::normal, bank_grp_infos);

    // create JPEG kernel runner instance
    std::cout << "Create runner" << std::endl;
    xrt::run run_jpeg(krnl_jpeg); 

    // set jpeg kernel arguments
    run_jpeg.set_arg(krnl_jpeg_jpeg_ptr,    jpeg_buffer);   
    run_jpeg.set_arg(krnl_jpeg_jpeg_size,   file_size);    
    run_jpeg.set_arg(krnl_jpeg_yuv_ptr,     yuv_buffer);    
    run_jpeg.set_arg(krnl_jpeg_infos_ptr,   infos_buffer);

    std::cout << "Read JPEG file and transfer JPEG data to device... ";
    read_file(file_name.c_str(), file_size, (char*)jpeg_data);  
    jpeg_buffer.write(jpeg_data);
    jpeg_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    std::cout << "Done" << std::endl;

    std::cout << "Run krnl_jpeg... " ;
    run_jpeg.start();
    run_jpeg.wait();
    std::cout << "Finish" << std::endl; 

    std::cout << "Transfer YUV data to host... ";
    yuv_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    yuv_buffer.read(yuv_data);
    std::cout << "Done" << std::endl;
    
    std::cout << "Transfer Info data to host... ";
    infos_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    infos_buffer.read(infos_data);
    std::cout << "Done" << std::endl;   

   // extract JPEG decoder return information
    xf::codec::cmp_info cmp_info[MAX_NUM_COLOR];
    xf::codec::img_info img_info;
    xf::codec::bas_info bas_info;

    // 0: decode jfif successful
    // 1: marker in jfif is not in expectation
    int rtn = 0;

    // 0: decode huffman successful
    // 1: huffman data is not in expectation
    int rtn2 = false;

    rebuild_infos(img_info, cmp_info, bas_info, rtn, rtn2, (uint32_t*)infos_data);

    FILE *info_fp;
    info_fp = fopen("info.dat", "w");
    for (int i = 0; i < 1024; i++)
    {
        fprintf(info_fp, "%08X\n", *(((uint32_t*)infos_data) + i));
    }

    if (rtn || rtn2) {
        printf("[ERROR]: Decoding the bad case input file!\n");
    if (rtn == 1) {
            printf("[code 1] marker in jfif is not in expectation!\n");
        } else if (rtn == 2) {
            printf("[code 2] huffman table is not in expectation!\n");
        } else {
            if (rtn2) {
                printf("[code 3] huffman data is not in expectation!\n");
            }
        }
        return 1;
    } 

    //xf::codec::COLOR_FORMAT fmt = bas_info->format;
    if ((bas_info.format != 3) || (bas_info.mcu_cmp != 3)) 
    {
        std::cout << "[ERROR] This example design requires JPEG file with YUV444 baseline format." << std::endl;
        return 1;
    }

    std::cout << "Successfully decode the JPEG file." << std::endl;

    rebuild_image(&bas_info, yuv_data);
    
    int image_height = bas_info.axi_height[0] * 8;
    int image_width  = bas_info.axi_width[0] * 8;
    int mcu_count    = img_info.hls_mcuc;


// --------------------------------------------------------------------------------------
// single color overlay using AIE with decoded image
// --------------------------------------------------------------------------------------

    // create host buffer for host-device data exchange
    uint8_t *rgb_data;     // host buffer for final output RGB data

    rgb_data   = new uint8_t [image_height * image_width * 4];  // RGBA format


    // get memory bank groups for device buffer
    xrtMemoryGroup bank_grp_rgb   = krnl_rgb_mover.group_id(krnl_rgb_mover_buffer_ptr);
    xrtMemoryGroup bank_grp_ctrl  = krnl_yuv_mover.group_id(krnl_yuv_mover_ctrl_ptr);

    // create buffer object
    xrt::bo rgb_buffer   = xrt::bo (device, image_height * image_width * 4, xrt::bo::flags::normal, bank_grp_rgb);
    xrt::bo ctrl_buffer  = xrt::bo (device, 64, xrt::bo::flags::normal, bank_grp_ctrl);

    // create kernel runner for data mover
    xrt::run run_yuv_mover(krnl_yuv_mover);
    xrt::run run_rgb_mover(krnl_rgb_mover);

    // compose and transfer AIE control packet
    overlay_S0_control overlay_ctrl;
    overlay_ctrl.IMG_W = image_width;
    overlay_ctrl.IMG_H = image_height;
    overlay_ctrl.OVL_R = overlay_color_red;
    overlay_ctrl.OVL_G = overlay_color_green;
    overlay_ctrl.OVL_B = overlay_color_blue;
    overlay_ctrl.MCU_N = mcu_count;

    std::cout << "Transfer AIE ctrl packet to device... ";
    ctrl_buffer.write(&overlay_ctrl);
    ctrl_buffer.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    std::cout << "Done" << std::endl;

    // setting yuv mover kernel argument
    run_yuv_mover.set_arg(krnl_yuv_mover_ctrl_size, 64);
    run_yuv_mover.set_arg(krnl_yuv_mover_ctrl_ptr, ctrl_buffer);
    run_yuv_mover.set_arg(krnl_yuv_mover_data_size, (image_height * image_width * 3));
    run_yuv_mover.set_arg(krnl_yuv_mover_data_ptr, yuv_buffer);

    // setting rgb mover kernel argument
    run_rgb_mover.set_arg(krnl_rgb_mover_img_height, image_height);
    run_rgb_mover.set_arg(krnl_rgb_mover_img_width, image_width);
    run_rgb_mover.set_arg(krnl_rgb_mover_buffer_ptr, rgb_buffer);

    // run rgb and yuv mover kernels
    std::cout << "Run krnl_yuv_mover and krnl_rgb_mover... " << std::endl;
    run_yuv_mover.start();
    run_rgb_mover.start();
    run_yuv_mover.wait();
    std::cout << "krnl_yuv_mover finish" << std::endl;
    run_rgb_mover.wait();    
    std::cout << "krnl_rgb_mover finish" << std::endl; 

    // transfer back mixed RGB data
    std::cout << "Transfer mixed RGB data to host... ";
    rgb_buffer.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    rgb_buffer.read(rgb_data);
    std::cout << "Done" << std::endl;

    // write pixel data
    write_bmp(rgb_data, "./mixed.bmp", image_width, image_height);
    std::cout << "The mixed image is written to 'mixed.bmp'." << std::endl;

    free(jpeg_data);
    free(infos_data);
    free(yuv_data);
    free(rgb_data);

    return 0;
}
