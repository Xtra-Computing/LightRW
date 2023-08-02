#include <stdio.h>

#include "graph.h"

Graph* createGraph(const std::string &gName, const std::string &mode) {
    Graph* gptr;
    std::string dir = mode;

    if (gName == "AM") {
        gptr = new Graph(dir + "amazon-2008.mtx");
    }
    else if (gName == "BB") {
        gptr = new Graph(dir + "web-baidu-baike.edges");
    }
    else if (gName == "FU") {
        gptr = new Graph(dir + "soc-flickr.ungraph.edges");
    }
    else if (gName == "G23") {
        gptr = new Graph(dir + "graph500-scale23-ef16_adj.edges");
    }
    else if (gName == "G24") {
        gptr = new Graph(dir + "graph500-scale24-ef16_adj.edges");
    }
    else if (gName == "G25") {
        gptr = new Graph(dir + "graph500-scale25-ef16_adj.edges");
    }
    else if (gName == "GG") {
        gptr = new Graph(dir + "web-Google.mtx");
    }
    else if (gName == "HD") {
        gptr = new Graph(dir + "web-hudong.edges");
    }
    else if (gName == "HW") {
        gptr = new Graph(dir + "ca-hollywood-2009.mtx");
    }
    else if (gName == "LJ") {
        gptr = new Graph(dir + "LiveJournal1.txt");
    }
    else if (gName == "MG") {
        gptr = new Graph(dir + "bio-mouse-gene.edges");
    }
    else if (gName == "PK") {
        gptr = new Graph(dir + "pokec-relationships.txt");
    }
    else if (gName == "TC") {
        gptr = new Graph(dir + "wiki-topcats.mtx");
    }
    else if (gName == "WT") {
        gptr = new Graph(dir + "wiki-Talk.txt");
    }
    else if (gName == "R19") {
        gptr = new Graph(dir + "rmat-19-32.txt");
    }
    else if (gName == "R21") {
        gptr = new Graph(dir + "rmat-21-32.txt");
    }
    else if (gName == "R24") {
        gptr = new Graph(dir + "rmat-24-16.txt");
    }
    else if (gName == "TW") {
        gptr = new Graph(dir + "soc-twitter-2010.mtx");
    }
    else if (gName == "WP") {
        gptr = new Graph(dir + "wikipedia-20070206.mtx");
    }
    else {
        gptr = new Graph("/data/graph_dataset/rmat-19-2.txt");
        std::cout << "load /data/graph_dataset/rmat-19-2.txt as default ..." << std::endl;
    }
    return gptr;
}


double getCurrentTimestamp(void) {
    timespec a;
    clock_gettime(CLOCK_MONOTONIC, &a);
    return (double(a.tv_nsec) * 1.0e-9) + double(a.tv_sec);
}

