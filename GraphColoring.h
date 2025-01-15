//
// Created by qiming on 24-12-10.
//

#ifndef GRAPH_COLORING_H_
#define GRAPH_COLORING_H_


#include <array>
#include <iosfwd>
#include <vector>
struct GraphColoring {
    using node = int;
    using edge = std::array<node, 2>;

    int               node_num;
    int               edge_num;
    int               color_num;
    std::vector<edge> edges;

    GraphColoring() noexcept : node_num(0), edge_num(0), color_num(0) {}
    GraphColoring(const GraphColoring &other);
    GraphColoring(GraphColoring &&other) noexcept;
    GraphColoring &operator=(const GraphColoring &other)     = default;
    GraphColoring &operator=(GraphColoring &&other) noexcept = default;
    ~GraphColoring() noexcept                                = default;
    friend std::ostream &operator<<(std::ostream &os, const GraphColoring &gc);
    friend std::istream &operator>>(std::istream &is, GraphColoring &gc);
};
;


#endif// GRAPH_COLORING_H_
