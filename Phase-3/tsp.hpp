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

struct Order {
    int id;
    int p;
    int d;
    double depDist;
};

struct DriverRoute {
    std::vector<int> route;
    std::vector<Order> orders;
};

struct Solution {
    std::vector<DriverRoute> drivers;
    double total_latency = 0;
};

class TSP_Result {
public:
    double time;
    std::vector<std::tuple<int,std::vector<int>,std::vector<int>>> assignments;
    TSP_Result(double time, std::vector<std::tuple<int,int,double>> assignments) : time(time), assignments(assignments) {};
};

class TSP {
public:
    TSP_RESULT solve(Graph& graph, std::vector<std::tuple<int,int,int>>& orders,std::pair<int,int> fleet);
    Solution TSP::greedy_build(const Graph& graph,const std::vector<std::tuple<int,int,int>>& orders,int numDrivers,int depot);
};

#endif