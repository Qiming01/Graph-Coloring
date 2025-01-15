//
// Created by qiming on 24-12-10.
//

#include "Solver.h"
#include "RandomGenerator.h"
#include "Solution.h"
#include <chrono>
#include <fstream>
#include <future>
#include <iostream>

#define MAX_ITER 140000

void Solver::solve(long long int time_limit)
{
    if (time_limit < 0)
    {
        time_limit = INT32_MAX;
    }
    std::chrono::steady_clock::time_point limit_end_time = std::chrono::steady_clock::now() + std::chrono::seconds(time_limit);

    qm::RandomGenerator random_generator(_seed);
#ifdef TABU
    Solution    solution(_gc, random_generator, init_type::greedy);
    LocalSearch localSearch(_gc, _seed);
    localSearch.search(solution, MAX_ITER);
    for (int c: solution.colors)
    {
        std::cout << c << "\n";
    }
    return;
#endif

    std::vector<Solution> population(4, Solution(_gc.node_num));
    for (int i = 0; i < 4; i++)
    {
        population[i].init(_gc, random_generator, init_type::GREEDY);
    }
    Solution                 best_solution  = population.at(0);
    uint64_t                 cycle          = 0;
    uint64_t                 generation     = 0;
    uint64_t                 best_iteration = 0;
    std::vector<Solution>    children       = {population.at(0), population.at(1)};
    std::vector<LocalSearch> ls(2, LocalSearch(_gc, _seed));


    std::atomic<bool> found_solution(false);
    while (!found_solution.load() && std::chrono::steady_clock::now() < limit_end_time)
    {
        if (population[0] == population[1])
        {
            if (population[1] != best_solution)
                population[1] = best_solution;
            else if (population[1] != population[3])
                population[1] = population[3];
            else
                population[1].crossover(Solution(_gc, random_generator, init_type::DSATUR), best_solution, _gc, random_generator);
        }
        children[0].crossover(population[0], population[1], _gc, random_generator);
        children[1].crossover(population[1], population[0], _gc, random_generator);

        std::vector<std::future<void>> futures;

        for (int i = 0; i < 2; ++i)
        {
            futures.emplace_back(std::async(std::launch::async, [&, i] {
                ls[i].search(children.at(i), MAX_ITER, found_solution);
            }));
        }

        for (auto &future: futures)
        {
            future.wait();
        }

        for (int i = 0; i < 2; i++)
        {
            population[i] = children[i];
        }
        // elite1 <- best(p1, p2, elite1)
        if (population[0].conflict_num < population[2].conflict_num)
        {
            population[2] = population[0];
        }
        if (population[1].conflict_num < population[2].conflict_num)
        {
            population[2] = population[1];
        }
        // best <- best(bestSolution, elite1)
        if (population[2].conflict_num < best_solution.conflict_num)
        {
            best_solution  = population[2];
            best_iteration = generation;
        }
        if (generation % 10 == 0)
        {
            cycle++;
#ifdef DEBUG
            std::clog << "cycle: " << cycle;
            std::clog << "\tp1: " << population[0].conflict_num;
            std::clog << "\tp2: " << population[1].conflict_num;
            std::clog << "\telite1: " << population[2].conflict_num;
            std::clog << "\tbest: " << best_solution.conflict_num << "(" << best_iteration << ")" << std::endl;
#endif
            population[0] = population[3];
            // elite2 <- elite1
            population[3] = population[2];

            // elite1 <- newSolution
            population[2].init(_gc, random_generator, init_type::RANDOM);
        }
        generation++;
#ifdef AUTO_SAVE
        if (generation % 100 == 0)
        {
            std::ofstream out("auto_save.txt");
            for (int i = 0; i < _gc.node_num; i++)
            {
                out << best_solution.colors[i] << ", ";
            }
            out << "\n";
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < _gc.node_num; j++)
                {
                    out << population[i].colors[j] << ", ";
                }
                out << "\n";
            }
            out.close();
            std::clog << "saved" << std::endl;
        }
#endif
    }
}

void LocalSearch::init()
{
    int node_num         = _gc.node_num;
    int color_num        = _gc.color_num;
    tabu_table           = new uint64_t *[node_num]();
    adjacent_color_table = new int32_t *[node_num]();
    adjacent_nodes.resize(node_num);
    for (int i = 0; i < node_num; i++)
    {
        tabu_table[i]           = new uint64_t[color_num];
        adjacent_color_table[i] = new int32_t[color_num];
    }
    for (const auto &edge: _gc.edges)
    {
        adjacent_nodes[edge[0]].push_back(edge[1]);
        adjacent_nodes[edge[1]].push_back(edge[0]);
    }
}
void LocalSearch::search(Solution &solution, uint64_t max_iterations, std::atomic<bool> &found_solution)
{
    iteration = 0;
    for (int i = 0; i < _gc.node_num; i++)
    {
        for (int j = 0; j < _gc.color_num; j++)
        {
            adjacent_color_table[i][j] = 0;
            tabu_table[i][j]           = 0;
        }
    }
    for (auto i = 0; i < _gc.node_num; i++)
    {
        for (auto adj_node: adjacent_nodes[i])
        {
            adjacent_color_table[adj_node][solution.colors[i]]++;
        }
    }
    best_conflict_num = solution.conflict_num;
    while (iteration < max_iterations)
    {
        if (found_solution.load())
        {
            std::clog << "Thread " << " stopped early.\n";
            return;// 提前退出
        }
        find_move(solution);
        make_move(solution, 1);
        if (solution.conflict_num == 0)
        {
            found_solution.store(true);
            for (int i = 0; i < _gc.node_num; i++)
            {
                std::cout << solution.colors[i] << "\n";
            }
            return;
        }


#ifdef DEBUGLS
        if (iteration % 100000 == 0)
        {
            std::clog << "iteration: " << iteration << " conflict_num: " << solution.conflict_num << std::endl;
        }

#endif
        iteration++;
    }
    solution.colors       = best_solution;
    solution.conflict_num = best_conflict_num;
}
void LocalSearch::find_move(Solution &solution)
{
    int                tabu_delta         = INT32_MAX;   // 禁忌移动的delta
    int                non_tabu_delta     = INT32_MAX;   // 非禁忌移动的delta
    std::array<int, 3> best_tabu_move     = {-1, -1, -1};// 最佳禁忌移动
    int                tabu_move_num      = 0;           // 禁忌移动的数量
    std::array<int, 3> best_non_tabu_move = {-1, -1, -1};// 最佳非禁忌移动
    int                non_tabu_move_num  = 0;           // 非禁忌移动的数量

    for (auto i = 0; i < solution.conflict_node_num(); i++)
    {
        auto node = solution.conflict_nodes_queue.conflict_nodes[i];
        for (auto j = 0; j < _gc.color_num; j++)
        {
            if (j != solution.colors[node])
            {
                auto move_delta = adjacent_color_table[node][j] - adjacent_color_table[node][solution.colors[node]];
                if (iteration < tabu_table[node][j])
                {
                    if (move_delta < tabu_delta)
                    {
                        tabu_delta     = move_delta;
                        best_tabu_move = {node, solution.colors[node], j};
                        tabu_move_num  = 1;
                    } else if (move_delta == tabu_delta)
                    {
                        tabu_move_num++;
                        if (_random_generator.rand(tabu_move_num) == 0)
                        {
                            best_tabu_move = {node, solution.colors[node], j};
                        }
                    }
                } else
                {
                    if (move_delta < non_tabu_delta)
                    {
                        non_tabu_delta     = move_delta;
                        best_non_tabu_move = {node, solution.colors[node], j};
                        non_tabu_move_num  = 1;
                    } else if (move_delta == non_tabu_delta)
                    {
                        non_tabu_move_num++;
                        if (_random_generator.rand(non_tabu_move_num) == 0)
                        {
                            best_non_tabu_move = {node, solution.colors[node], j};
                        }
                    }
                }
            }
        }
    }

    if (tabu_delta < best_conflict_num - solution.conflict_num && tabu_delta < non_tabu_delta)
    {
        move = {best_tabu_move.at(0), best_tabu_move.at(1), best_tabu_move.at(2), tabu_delta};
    } else
    {
        move = {best_non_tabu_move.at(0), best_non_tabu_move.at(1), best_non_tabu_move.at(2), non_tabu_delta};
    }
}
void LocalSearch::make_move(Solution &solution, double lambda)
{
    // 更新冲突数
    solution.conflict_num += move.at(3);
    if (solution.conflict_num < best_conflict_num)
    {
        best_conflict_num = solution.conflict_num;
        best_solution     = solution.colors;
    }

    if (adjacent_color_table[move.at(0)][move.at(2)] == 0)
    {
        solution.conflict_nodes_queue.remove(move.at(0));
    } else
    {
        solution.conflict_nodes_queue.add(move.at(0));
    }

    // 更新邻接颜色表
    for (int adjNode: adjacent_nodes[move.at(0)])
    {
        // 如果新颜色是邻接结点的颜色，且邻接结点不在冲突结点中，则将邻接结点加入冲突结点
        if (move.at(2) == solution.colors[adjNode] && solution.conflict_nodes_queue.conflict_nodes_pos[adjNode] == -1)
        {
            solution.conflict_nodes_queue.add(adjNode);
        }

        adjacent_color_table[adjNode][move.at(1)]--;
        adjacent_color_table[adjNode][move.at(2)]++;

        // 如果旧颜色是邻接结点的颜色，且邻接结点的旧颜色冲突数量为0，则将邻接结点从冲突结点中移除
        if (move.at(1) == solution.colors[adjNode] && adjacent_color_table[adjNode][move.at(1)] == 0)
        {
            solution.conflict_nodes_queue.remove(adjNode);
        }
    }
    // 更新禁忌表
    tabu_table[move.at(0)][move.at(1)] = iteration + uint64_t(lambda * solution.conflict_num + _random_generator.rand(10));
    // 更新解
    solution.colors[move.at(0)] = move.at(2);
}
[[maybe_unused]] std::pair<std::array<int, 4>, std::array<int, 4>> LocalSearch::find_from_conflict_nodes(const Solution &solution, int start, int end)
{
    int                tabu_delta         = INT32_MAX;              // 禁忌移动的delta
    int                non_tabu_delta     = INT32_MAX;              // 非禁忌移动的delta
    std::array<int, 4> best_tabu_move     = {-1, -1, -1, INT32_MAX};// 最佳禁忌移动
    int                tabu_move_num      = 0;                      // 禁忌移动的数量
    std::array<int, 4> best_non_tabu_move = {-1, -1, -1, INT32_MAX};// 最佳非禁忌移动
    int                non_tabu_move_num  = 0;                      // 非禁忌移动的数量
                                                                    // 遍历冲突结点
    for (auto i = start; i < end; i++)
    {
        auto node = solution.conflict_nodes_queue.conflict_nodes[i];
        for (auto j = 0; j < _gc.color_num; j++)
        {
            if (j != solution.colors[node])
            {
                auto move_delta = adjacent_color_table[node][j] - adjacent_color_table[node][solution.colors[node]];
                if (iteration < tabu_table[node][j])
                {
                    if (move_delta < tabu_delta)
                    {
                        tabu_delta     = move_delta;
                        best_tabu_move = {node, solution.colors[node], j, move_delta};
                        tabu_move_num  = 1;
                    } else if (move_delta == tabu_delta)
                    {
                        tabu_move_num++;
                        if (_random_generator.rand(tabu_move_num) == 0)
                        {
                            best_tabu_move = {node, solution.colors[node], j, move_delta};
                        }
                    }
                } else
                {
                    if (move_delta < non_tabu_delta)
                    {
                        non_tabu_delta     = move_delta;
                        best_non_tabu_move = {node, solution.colors[node], j, move_delta};
                        non_tabu_move_num  = 1;
                    } else if (move_delta == non_tabu_delta)
                    {
                        non_tabu_move_num++;
                        if (_random_generator.rand(non_tabu_move_num) == 0)
                        {
                            best_non_tabu_move = {node, solution.colors[node], j, move_delta};
                        }
                    }
                }
            }
        }
    }
    return {best_tabu_move, best_non_tabu_move};
}
void LocalSearch::search(Solution &solution, uint64_t max_iterations)
{
    iteration = 0;
    for (int i = 0; i < _gc.node_num; i++)
    {
        for (int j = 0; j < _gc.color_num; j++)
        {
            adjacent_color_table[i][j] = 0;
            tabu_table[i][j]           = 0;
        }
    }
    for (auto i = 0; i < _gc.node_num; i++)
    {
        for (auto adj_node: adjacent_nodes[i])
        {
            adjacent_color_table[adj_node][solution.colors[i]]++;
        }
    }
    best_conflict_num = solution.conflict_num;
    while (iteration < max_iterations)
    {
        find_move(solution);
        make_move(solution, 1);
        if (solution.conflict_num == 0)
        {
            break;
        }


#ifdef DEBUGLS
        if (iteration % 100000 == 0)
        {
            std::clog << "iteration: " << iteration << " conflict_num: " << solution.conflict_num << std::endl;
        }

#endif
        iteration++;
    }
}