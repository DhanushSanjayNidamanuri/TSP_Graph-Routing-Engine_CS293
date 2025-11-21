#ifndef TSP_HPP
#define TSP_HPP

#include "graph.hpp"
#include <vector>
#include <string>
#include <queue> 
#include <map>
#include <algorithm>
class Graph;
class Node;
class TSP_Result {
public:
    double time;
    std::vector<std::tuple<int,std::vector<int>,std::vector<int>>> assignments;
    TSP_Result(double time, std::vector<std::tuple<int,int,double>> assignments) : time(time), assignments(assignments) {};
};

class TSP {
public:
    TSP_RESULT solve(Graph& graph, std::vector<std::tuple<int,int,int>>& orders,std::pair<int,int> fleet);
};

#endif