//
// Created by qiming on 24-12-10.
//

#ifndef SOLUTION_H
#define SOLUTION_H

#include "GraphColoring.h"
#include "RandomGenerator.h"
#include <vector>
struct ConflictNodes {
    explicit ConflictNodes(int node_num) : conflict_nodes_pos(node_num, -1) {}

    std::vector<int> conflict_nodes;
    std::vector<int> conflict_nodes_pos;

    void add(int node);
    void remove(int node);
    void clear()
    {
        conflict_nodes.clear();
        conflict_nodes_pos.assign(conflict_nodes_pos.size(), -1);
    }

    [[nodiscard]] int size() const { return (int) conflict_nodes.size(); }

    [[maybe_unused]] [[nodiscard]] bool empty() const { return conflict_nodes.empty(); }
};

enum init_type
{
    RANDOM,
    GREEDY,
    DSATUR,
};

struct Solution {
    int              conflict_num;
    std::vector<int> colors;
    ConflictNodes    conflict_nodes_queue;
    explicit Solution(int node_num) : conflict_num(INT32_MAX), conflict_nodes_queue(node_num), colors(node_num) {}
    Solution(const GraphColoring &gc, qm::RandomGenerator &rg, init_type type = init_type::RANDOM);
    Solution &operator=(const Solution &other)     = default;
    Solution &operator=(Solution &&other) noexcept = default;
    Solution(const Solution &other)                = default;
    void init(const GraphColoring &gc, qm::RandomGenerator &rg, init_type type = init_type::RANDOM);

    Solution(const GraphColoring &gc, const std::vector<int> &colors) : conflict_num(INT32_MAX), conflict_nodes_queue(gc.node_num), colors(colors) { calculate_conflicts(gc); }

    bool operator==(const Solution &other) const;

    [[nodiscard]] int conflict_node_num() const { return static_cast<int>(conflict_nodes_queue.size()); }

    void calculate_conflicts(const GraphColoring &gc);

    void crossover(const Solution &parent1, const Solution &parent2, const GraphColoring &gc, qm::RandomGenerator rg);
};


#endif// SOLUTION_H
