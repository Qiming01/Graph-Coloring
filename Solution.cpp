//
// Created by qiming on 24-12-10.
//

#include "Solution.h"
#include <algorithm>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>
void ConflictNodes::add(int node)
{
    if (conflict_nodes_pos[node] == -1)
    {
        conflict_nodes.push_back(node);
        conflict_nodes_pos[node] = static_cast<int>(conflict_nodes.size()) - 1;
    }
}
void ConflictNodes::remove(int node)
{
    if (conflict_nodes_pos[node] != -1)
    {
        conflict_nodes[conflict_nodes_pos[node]] = conflict_nodes.back();
        conflict_nodes.pop_back();
        conflict_nodes_pos[conflict_nodes[conflict_nodes_pos[node]]] = conflict_nodes_pos[node];

        conflict_nodes_pos[node] = -1;
    }
}


void greedy_init(Solution *solution, const GraphColoring &gc, qm::RandomGenerator &rg)
{
    // 获取结点的邻接结点
    std::vector<std::vector<int>> adjacencyList(gc.node_num);
    for (const auto &edge: gc.edges)
    {
        adjacencyList[edge[0]].push_back(edge[1]);
        adjacencyList[edge[1]].push_back(edge[0]);
    }

    // 将节点按照度数递减顺序排序
    std::vector<int> nodeOrder(gc.node_num);
    for (int i = 0; i < gc.node_num; ++i)
    {
        nodeOrder[i] = i;
    }

    // 按邻居数量排序（度数递减）
    std::sort(nodeOrder.begin(), nodeOrder.end(), [&](int a, int b) {
        return adjacencyList[a].size() > adjacencyList[b].size();
    });

    // 为每个节点分配颜色
    for (int node: nodeOrder)
    {
        std::unordered_set<int> forbiddenColors;// 存放相邻节点使用的颜色

        // 找到相邻节点已经使用的颜色
        for (int neighbor: adjacencyList[node])
        {
            if (solution->colors[neighbor] != -1)
            {
                forbiddenColors.insert(solution->colors[neighbor]);
            }
        }

        int start = rg.rand(gc.color_num);
        for (int i = 0; i < gc.color_num; ++i)
        {
            int color = (i + start) % gc.color_num;
            if (forbiddenColors.find(color) == forbiddenColors.end())
            {
                solution->colors[node] = color;
                break;
            }
        }

        // 如果所有颜色都冲突，随机选择一个颜色（允许少量冲突）
        if (solution->colors[node] == -1)
        {
            solution->colors[node] = rg.rand(gc.color_num);
        }
    }
}

void DSATUR_init(Solution *solution, const GraphColoring &gc, qm::RandomGenerator &rg)
{
    int n           = gc.node_num;
    int color_count = gc.color_num;
    // Adjacency list using vector
    std::vector<std::vector<int>> adjacency_list(n);
    for (const auto &edge: gc.edges)
    {
        int u = edge[0], v = edge[1];
        adjacency_list[u].push_back(v);
        adjacency_list[v].push_back(u);
    }

    // Data structures
    std::vector<int>               colors(n, -1);                                        // Colors assigned to vertices
    std::vector<int>               saturation(n, 0);                                     // Saturation degree of each vertex
    std::vector<int>               degree(n, 0);                                         // Degree of each vertex
    std::vector<std::vector<bool>> used_colors(n, std::vector<bool>(color_count, false));// Used colors per vertex

    // Initialize degrees
    for (int i = 0; i < n; ++i)
    {
        degree[i] = (int) adjacency_list[i].size();
    }

    // Priority queue for vertex selection: (saturation, degree, vertex_id)
    auto compare = [](const std::array<int, 3> &a, const std::array<int, 3> &b) {
        if (a[0] != b[0]) return a[0] < b[0];// Higher saturation first
        if (a[1] != b[1]) return a[1] < b[1];// Higher degree second
        return a[2] > b[2];                  // Lower vertex_id third
    };
    std::priority_queue<std::array<int, 3>, std::vector<std::array<int, 3>>, decltype(compare)> pq(compare);

    for (int i = 0; i < n; ++i)
    {
        pq.push({0, degree[i], i});
    }

    // Main loop: assign colors to vertices
    while (!pq.empty())
    {
        std::array<int, 3> top = pq.top();
        int                u   = top[2];// Extract the third element (vertex_id)
        pq.pop();

        // Skip if already colored
        if (colors[u] != -1) continue;

        // Find the smallest available color for u
        for (int c = 0; c < color_count; ++c)
        {
            if (!used_colors[u][c])
            {
                colors[u] = c;
                break;
            }
        }

        // Fail if no valid color is found
        if (colors[u] == -1)
        {
            colors[u] = rg.rand(color_count);
        }

        // Update saturation and color usage for neighbors
        for (int neighbor: adjacency_list[u])
        {
            if (colors[neighbor] == -1)
            {
                if (!used_colors[neighbor][colors[u]])
                {
                    saturation[neighbor]++;
                    used_colors[neighbor][colors[u]] = true;
                    pq.push({saturation[neighbor], degree[neighbor], neighbor});// Update priority
                }
            }
        }
    }
    solution->colors = colors;
}
Solution::Solution(const GraphColoring &gc, qm::RandomGenerator &rg, init_type type) : colors(gc.node_num, -1),
                                                                                       conflict_nodes_queue(gc.node_num), conflict_num(INT32_MAX)
{
    init(gc, rg, type);
}


// 函数用于判断两个着色方案是否等价
bool are_equivalent(const std::vector<int> &color1, const std::vector<int> &color2)
{
    int node_num = (int) color1.size();
    // 使用map来记录颜色到结点集合的映射
    std::unordered_map<int, std::unordered_set<int>> colorMap1, colorMap2;

    // 遍历color1，构造颜色到结点的映射
    for (int i = 0; i < node_num; ++i)
    {
        colorMap1[color1[i]].insert(i);
    }

    // 遍历color2，构造颜色到结点的映射
    for (int i = 0; i < node_num; ++i)
    {
        colorMap2[color2[i]].insert(i);
    }

    // 判断两个颜色映射是否等价
    if (colorMap1.size() != colorMap2.size()) return false;// 不同颜色种类的数量不相等

    // 比较每个颜色对应的结点集合是否一致
    for (const auto &entry: colorMap1)
    {
        bool foundEquivalent = false;
        for (const auto &entry2: colorMap2)
        {
            if (entry.second == entry2.second)
            {
                foundEquivalent = true;
                break;
            }
        }
        if (!foundEquivalent) return false;
    }
    return true;
}

bool Solution::operator==(const Solution &other) const
{
    return are_equivalent(colors, other.colors);
}
void Solution::calculate_conflicts(const GraphColoring &gc)
{
    conflict_nodes_queue.clear();
    conflict_num = 0;
    for (const auto &edge: gc.edges)
    {
        if (colors[edge[0]] == colors[edge[1]])
        {
            conflict_num++;
            conflict_nodes_queue.add(edge[0]);
            conflict_nodes_queue.add(edge[1]);
        }
    }
}
void Solution::init(const GraphColoring &gc, qm::RandomGenerator &rg, init_type type)
{
    switch (type)
    {
        case greedy: {
            greedy_init(this, gc, rg);
            break;
        }
        case random: {
            for (int i = 0; i < gc.node_num; ++i)
            {
                colors[i] = rg.rand(gc.color_num);
            }
            break;
        }
        case DSATUR: {
            DSATUR_init(this, gc, rg);
            break;
        }
    }
    conflict_num = 0;
    calculate_conflicts(gc);
}
void Solution::crossover(const Solution &parent1, const Solution &parent2, const GraphColoring &gc, qm::RandomGenerator rg)
{
    std::vector<std::vector<int>> parents = {parent1.colors, parent2.colors};
    std::vector<std::vector<int>> each_color_num(2);
    for (auto i = 0; i < 2; i++)
    {
        each_color_num[i].resize(gc.color_num, 0);
        for (auto j = 0; j < gc.node_num; j++)
        {
            each_color_num[i][parents[i][j]]++;
        }
    }
    for (auto &c: colors)
    {
        c = -1;
    }
    for (int i = 0; i < gc.color_num; i++)
    {
        int index = i & 1;
        // 选择颜色最多的颜色
        int valMax   = -1;
        int colorMax = -1;
        int startInd = rg.rand(gc.color_num);
        int maxNum   = 0;
        // int startInd = 0;
        for (int j = 0; j < gc.color_num; j++)
        {
            if (each_color_num[index][(j + startInd) % gc.color_num] > valMax)
            {
                valMax   = each_color_num[index][(j + startInd) % gc.color_num];
                colorMax = (j + startInd) % gc.color_num;
                maxNum   = 1;
            } else if (each_color_num[index][(j + startInd) % gc.color_num] == valMax)
            {
                maxNum++;
                if (rg.rand(maxNum) == 0)
                {
                    colorMax = (j + startInd) % gc.color_num;
                }
            }
        }
        // 分配颜色
        for (int j = 0; j < gc.node_num; j++)
        {
            if (parents[index][j] == colorMax && this->colors[j] == -1)
            {
                this->colors[j] = i;
                for (int k = 0; k < 2; k++)
                {
                    each_color_num[k][parents[k][j]]--;
                }
            }
        }
    }
    // 未分配颜色的结点随机分配颜色
    for (int i = 0; i < gc.node_num; i++)
    {
        if (this->colors[i] == -1)
        {
            this->colors[i] = rg.rand(gc.color_num);
        }
    }
    calculate_conflicts(gc);
}
