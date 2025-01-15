#include "GraphColoring.h"
#include "RandomGenerator.h"
#include "Solver.h"
#include <chrono>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <time_out> <seed>" << std::endl;
        return EXIT_FAILURE;
    }
    GraphColoring gc;
    std::cin >> gc;
    long long time_out = atoll(argv[1]);
    int       seed     = atoi(argv[2]);
#ifdef DEBUG
    std::clog << "Node num\t" << gc.node_num << std::endl;
    std::clog << "Edge num\t" << gc.edge_num << std::endl;
    std::clog << "Color num\t" << gc.color_num << std::endl;
    std::clog << "Limit time\t" << time_out << "s" << std::endl;
    std::clog << "Random seed\t" << seed << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
#endif
    Solver(std::move(gc), seed).solve(time_out);
#ifdef DEBUG
    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::clog << "Time used " << duration.count() << " milliseconds to execute." << std::endl;
#endif
    return 0;
}
