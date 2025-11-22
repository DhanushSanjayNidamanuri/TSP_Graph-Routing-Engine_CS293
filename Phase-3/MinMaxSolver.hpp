#ifndef MIN_MAX_SOLVER_HPP
#define MIN_MAX_SOLVER_HPP

#include "graph.hpp"
#include <nlohmann/json.hpp>
#include <vector>
#include <tuple>

class MinMaxSolver{
    public:
        static nlohmann::json solve(Graph& graph, const nlohmann::json& query);
    private:
        static double calculate_time(Graph& graph, std::vector<int>& route);
        static std::vector<std::tuple<int, std::vector<int>, std::vector<int>>> solve_min_max(Graph& graph, std::vector<std::tuple<int, int, int>>& orders, int num_drivers, int depot);
};

#endif