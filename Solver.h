//
// Created by qiming on 24-12-10.
//

#ifndef SOLVER_H
#define SOLVER_H

#include "GraphColoring.h"
#include "RandomGenerator.h"
#include "Solution.h"
#include <atomic>

class Solver
{
public:
    explicit Solver(GraphColoring &&gc, int seed = 2001) noexcept : _gc(std::move(gc)), _seed(seed) {}
    void solve(long long time_limit = -1);

private:
    GraphColoring _gc;
    int           _seed;
};


class LocalSearch
{
public:
    LocalSearch(const GraphColoring &gc, int seed) : _gc(gc), _random_generator(seed), iteration(0), best_conflict_num(INT32_MAX)
    {
        init();
    }
    LocalSearch(const LocalSearch &other)
    {
        _gc               = other._gc;
        _random_generator = other._random_generator;
        iteration         = other.iteration;
        best_conflict_num = other.best_conflict_num;
        init();
    }
    void search(Solution &solution, uint64_t max_iterations, std::atomic<bool> &found_solution);
    void search(Solution &solution, uint64_t max_iterations);
    ~LocalSearch()
    {
        for (int i = 0; i < _gc.node_num; i++)
        {
            delete[] tabu_table[i];
        }
        delete[] tabu_table;

        for (int i = 0; i < _gc.node_num; i++)
        {
            delete[] adjacent_color_table[i];
        }
        delete[] adjacent_color_table;
    }


private:
    void init();
    void find_move(Solution &solution);
    void make_move(Solution &solution, double lambda);

    [[maybe_unused]] std::pair<std::array<int, 4>, std::array<int, 4>> find_from_conflict_nodes(const Solution &solution, int start, int end);

private:
    GraphColoring       _gc;
    qm::RandomGenerator _random_generator;

    int64_t            iteration;
    int                best_conflict_num;
    std::array<int, 4> move;

    std::vector<std::vector<int>> adjacent_nodes;

    std::vector<int> best_solution;

    uint64_t **tabu_table           = nullptr;
    int32_t  **adjacent_color_table = nullptr;
};

#endif// SOLVER_H
