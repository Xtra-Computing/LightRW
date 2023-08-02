[![GitHub license](https://img.shields.io/badge/license-apache2-yellowgreen)](./LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/Xtra-Computing/LightRW.svg)](https://github.com/Xtra-Computing/LightRW/issues)


# LightRW: FPGA Accelerated Graph Dynamic Random Walks

## Table of Contents

<!-- MarkdownTOC -->

- [Introduction](#introduction)
- [Getting Started](#getting-started)
    - [System Requirements and Dependencies](#system-requirements-and-dependencies)
    - [Setting Up the Compilation Environment](#setting-up-the-compilation-environment)
        - [Installing Vitis](#installing-vitis)
        - [Installing FPGA Runtime](#installing-fpga-runtime)
        - [Installing FPGA Shell](#installing-fpga-shell)
        - [Installing RABS Dependency](#installing-rabs-dependency)
    - [Compiling LightRW](#compiling-lightrw)
        - [Cloning the Repository with Submodules](#cloning-the-repository-with-submodules)
        - [Understanding the Code Structure](#understanding-the-code-structure)
        - [Building LightRW](#building-lightrw)
    - [Running LightRW](#running-lightrw)
        - [Program Overview](#program-overview)
        - [Preparing Datasets](#preparing-datasets)
        - [Downloading Formatted Datasets](#downloading-formatted-datasets)
        - [Programming the Secondary Shell](#programming-the-secondary-shell)
        - [Executing the Program](#executing-the-program)
        - [Performance Log](#performance-log)
        - [Running All Tests](#running-all-tests)
- [Acknowledging Our Work](#acknowledging-our-work)
- [Getting in Touch](#getting-in-touch)

<!-- /MarkdownTOC -->


## Introduction

This repository contains the source code for LightRW, a project that accelerates graph dynamic random walks using AMD FPGAs. LightRW utilizes the proposed efficiency parallel weighted reservoir sampling algorithm and its hardware implementation to speed up random walk algorithms that have run-time varying transition probabilities. For more details, please refer to our [paper](doc/lightrw.pdf).

LightRW supports MetaPath and Node2Vec random walk algorithms, and the implementations are based on AMD [Alveo U250](https://www.xilinx.com/products/boards-and-kits/alveo/u250.html) FPGAs. This documentation provides the fundamental instructions to compile and execute LightRW.




## Getting Started


### System Requirements and Dependencies

To ensure the smooth operation of our code, we have outlined the necessary system requirements and dependencies below. Please ensure your environment aligns with the following specifications:


|Dependency| Description|
|--------------|--------------|
|OS| Ubuntu 20.04|
| Linux Kernel: | 5.4.x | 
| FPGA Platform:| AMD Alveo U250|
| FPGA Development: | Vitis Core Development Kit 2020.2 |
| FPGA Shell:| xilinx_u250_gen3x16_xdma_3_1_202020_1 (modified)|
| FPGA runtime:| XRT 2.14.354 |

**NOTE*: Please note that the FPGA Shell has been modified to suit the specific needs of our project. Ensure you have the correct version with the following instructions.


### Setting Up the Compilation Environment

The official U250 shell limits the number of FPGA kernels that can be implemented. To accommodate our design, we have modified the official U250 shell. The following instructions will guide you through setting up a development environment that can reproduce our implementations.

We recommend using the [HACC@NUS cluster](https://xacchead.d2.comp.nus.edu.sg), as it already has all the required environment set up.


#### Installing Vitis

1. Download and install the Vitis Core Development Kit 2020.2 from the AMD official [website](https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive.html).
2. After installation, execute the following script to install the necessary dependencies:

```bash
sudo /path/to/Xilinx/Vitis/2020.2/scripts/installLibs.sh
```

3. Follow [this guide](https://support.xilinx.com/s/article/76960?language=en_US) to patch the Vivado toolchain to ensure the export IP functionality works correctly.

#### Installing FPGA Runtime

Download and install the Xilinx Runtime Library (XRT) from the AMD official website using the following command:

```bash
wget -O xrt.deb https://www.xilinx.com/bin/public/openDownload?filename=xrt_202220.2.14.354_20.04-amd64-xrt.deb
sudo apt install ./xrt.deb
```

**Note:** Higher versions of the Linux kernel may result in a failed installation of XRT.



#### Installing FPGA Shell

AMD/Xilinx currently does not provide a download link for the shell `xilinx_u250_gen3x16_xdma_3_1_202020_1`. We have shared the related files, which you can install using the following commands:

```bash
wget -O shell.tar.gz  https://www.dropbox.com/s/rnf7z219xijch3h/xilinx-u250-gen3x16-xdma-all_3.1-3063142.deb_2.tar.gz?dl=0
tar -zxvf shell.tar.gz
wget -O dev.deb  https://www.dropbox.com/s/zly45955gexip9j/xilinx-u250-gen3x16-xdma-3.1-202020-1-dev_1-3061241_all.deb?dl=0
sudo apt install *.deb 
```

We have also modified the official development shell to support a larger number of kernels that can be deployed on a single FPGA. To use our modified shell:

```bash
wget -O hw.xsa  https://www.dropbox.com/s/sks6o254lflt7qo/hw.xsa?dl=0
sudo cp /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_3_1_202020_1/hw/hw.xsa /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_3_1_202020_1/hw/hw_backup.xsa
sudo cp hw.xsa /opt/xilinx/platforms/xilinx_u250_gen3x16_xdma_3_1_202020_1/hw/hw.xsa
```


#### Installing RABS Dependency

LightRW uses the Rule-Based Accelerator Building System (RABS) on top of Vitis to build accelerators. To install the required packages:

```bash
sudo apt install graphviz libgraphviz-dev faketime opencl-headers
pip install numpy
pip install graphviz
```


### Compiling LightRW

#### Cloning the Repository with Submodules

To clone the repository along with its submodules, use the following command:

```bash
git clone --recurse-submodules  git@github.com:Xtra-Computing/LightRW.git
```

#### Understanding the Code Structure

The repository is structured as follows:

``` shell 
├── app        # Contains the configurations for the accelerator project. Each subfolder corresponds to a hardware accelerator.
│   ├── test   # Contains the configurations for unit tests or benchmark projects.
├── host       # Contains the CPU code for preparing data and controlling the FPGA accelerator.
├── src        # Contains the FPGA source code, grouped in modules.
├── test       # Contains the CPU/FPGA test code.
├── lib        # Contains libraries of rules to use as a subgroup of kernels.
├── misc       # Contains scripts for automating tests.
└── mk         # Contains the RABS submodule for building the accelerator.
```

#### Building LightRW

To build LightRW, use the `make` command as shown below. Replace `${app_name}` with the name of the application in the `app` directory:

```bash
make app=${app_name} TARGET=hw all 
```

Here's a quick guide to the arguments:

- `app`: Specifies the target accelerator to be built. 
- `TARGET`: 
    + `hw`: Builds an accelerator that can run on real FPGA hardware.
    + `hw_emu`: Builds a project that can run waveform-based simulation.
    + The default value can be found in the corresponding `kernel.mk` files.
- `all`: Builds the entire project.

For example, to build a full implementation of the MetaPath random walk accelerator, use:

```bash
make app=metadata_x4 TARGET=hw all 
```

To build a full implementation of the Node2Vec random walk accelerator, use:

```bash
make app=node2vec_x4 TARGET=hw all  
```

Please note that the build of each accelerator may take around 15 hours, depending on the server's performance.

To build tests and benchmarks (projects in `app/test/`), replace `${test_name}` with the name of the test. For example:

```bash
make test=vcache_test TARGET=hw all 
```


### Running LightRW

#### Program Overview

A successful build will generate the program and FPGA bitstream. Here is an example of the `metapath_x4` build:

```base
├── metapath_x4.app                      # CPU program.
├── build_dir_metapath_metapath_x4
│   ├── clock.log                        # Accelerator clock.
│   ├── git
│   ├── kernel.cfg.json.pdf              # Accelerator kernel topology.
│   ├── kernel.link.ltx
│   ├── kernel.link.xclbin.info
│   ├── kernel.link.xclbin.link_summary
│   ├── kernel.xclbin                    # FPGA bitstream. 
│   ├── kernel.xclbin.package_summary
│   ├── link                             # FPGA build log.
│   ├── report                           # FPGA report.
└── xrt.ini
```

RABS will generate the kernel topology for debugging. You can view the example topologies of single channel MetaPath and Node2Vec here: [MetaPath Topo](doc/metapath.pdf) and [Node2Vec Topo](doc/node2vec.pdf).

#### Preparing Datasets

LightRW accepts graphs in the same format as ThunderRW. You can follow the `prepare_data.sh` script [here](https://github.com/Xtra-Computing/ThunderRW/blob/main/prepare_data.sh) to prepare the input graphs, or you can download them in the next section.

#### Downloading Formatted Datasets

You can download the formatted graphs [here](https://www.dropbox.com/scl/fo/1jk8fllupfxvpvdbsdfip/h?rlkey=rxffrugutahykjomiu1vq5qyr&dl=0). These can be used directly as the input for our accelerator.

#### Programming the Secondary Shell

The U250 requires programming a secondary shell:

```bash
sudo /opt/xilinx/xrt/bin/xbmgmt partition --program --name xilinx_u250_gen3x16_xdma_shell_3_1 --card ${PCIE_ID}
```

Here, `${PCIE_ID}` is the PCIE BDF id of the FPGA board.

#### Executing the Program

To execute the program, use the following command, for example the `${metapath_x4}`

```bash
./metapath_x4.app  -fpga build_dir_metapath_metapath_x4/kernel.xclbin -graph ${path/to/formatted/graph}
```

The arguments are as follows:

- `fpga`: The `kernel.xclbin` file used to configure the FPGAs.
- `graph`: The path that stores the graph dataset.

#### Performance Log

The output will look something like this:

```bash
...
[END] fpga init:  0.010874
[END] pcie memory copy: 1.721860
[END] acc execution:  4.039499 
[END] overall thr (M Step/s): 36.721491
```

This output displays the elapsed time of accelerator initialization, data transfer between CPU and FPGA through PCIE, and accelerator execution. The accelerator execution time is the median value of ten measurements.

#### Running All Tests

We provide a script to run the throughput test on all graphs in the `misc` directory:

```bash
./misc  ${app_name} ${path_to_all_graphs}
```

Ensure all graphs have been downloaded in `${path_to_all_graphs}`. The `${app_name}` is the same as the input used during the build process. For example:

```bash
./misc  metapath_x4 /data/graphs/
```

The script will generate a log directory with a timestamp (e.g.,`${log_2023_08_02_11_40_59}`), and all execution logs will be stored in this directory.


## Acknowledging Our Work

We are delighted that you have found our repository beneficial for your research. We kindly request that you acknowledge our work by citing our research paper. Below is the citation in BibTeX format for your convenience:

```bibtex
@article{10.1145/3588944,
author = {Tan, Hongshi and Chen, Xinyu and Chen, Yao and He, Bingsheng and Wong, Weng-Fai},
title = {LightRW: FPGA Accelerated Graph Dynamic Random Walks},
year = {2023},
issue_date = {May 2023},
publisher = {Association for Computing Machinery},
address = {New York, NY, USA},
volume = {1},
number = {1},
url = {https://doi.org/10.1145/3588944},
doi = {10.1145/3588944},
journal = {Proc. ACM Manag. Data},
month = {may},
articleno = {90},
numpages = {27},
}
```

## Getting in Touch

This repository is a prototype for an accelerator, and we are actively working on extending its support to more FPGA platforms and large graph workloads. We are committed to keeping this repository updated in sync with our development repositories.

We appreciate your interest and encourage you to share your experiences and issues while using this repository. Please feel free to submit issues directly on this platform. For more specific concerns or queries, you can reach out to Hongshi Tan via email at hongshan@u.nus.edu. We look forward to hearing from you and improving our work based on your valuable feedback.
