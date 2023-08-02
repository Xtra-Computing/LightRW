#ifndef __LIGHTRW_METAPATH_H__
#define __LIGHTRW_METAPATH_H__


#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <iostream>
#include <vector>

#include <unistd.h>
#include <sys/stat.h>


#include "graph.h"
#include "util/command_parser.h"
#include "log.h"
#include "pcg_basic.h"


#include "ring_manager.h"
#include "step_loader.h"

#ifdef HW_VCACHE
#include "vcache_loader.h"
#else
#include "vertex_loader.h"
#endif

#include "fetch_next.h"

#include "step_counter.h"






#endif __LIGHTRW_METAPATH_H__