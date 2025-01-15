//
// Created by qiming on 24-12-10.
//

#include "GraphColoring.h"
#include <iostream>
std::ostream &operator<<(std::ostream &os, const GraphColoring &gc)
{
    os << gc.node_num << " " << gc.edge_num << " " << gc.color_num << std::endl;
    for (auto e: gc.edges)
        os << e.at(0) << " " << e.at(1) << std::endl;
    return os;
}

std::istream &operator>>(std::istream &is, GraphColoring &gc)
{
    is >> gc.node_num >> gc.edge_num >> gc.color_num;
    gc.edges.resize(gc.edge_num);
    for (auto &e: gc.edges)
        is >> e[0] >> e[1];
    return is;
}
GraphColoring::GraphColoring(const GraphColoring &other)
{
    node_num  = other.node_num;
    edge_num  = other.edge_num;
    color_num = other.color_num;
    edges     = other.edges;
}

GraphColoring::GraphColoring(GraphColoring &&other) noexcept
{
    node_num  = other.node_num;
    edge_num  = other.edge_num;
    color_num = other.color_num;
    edges     = std::move(other.edges);
}