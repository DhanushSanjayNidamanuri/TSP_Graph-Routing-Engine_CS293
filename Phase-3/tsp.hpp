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
    int pickup;
    int dropoff;
    double time_2_reach;
    Order(){};
    Order(int id,int pickup,int dropoff,double time_2_reach):id(id),pickup(pickup),dropoff(dropoff),time_2_reach(time_2_reach){};
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
    TSP_Result(){};
    TSP_Result(double time, std::vector<std::tuple<int,std::vector<int>,std::vector<int>>> assignments) : time(time), assignments(assignments) {};
};

class TSP {
public:
    TSP_Result solve(Graph& graph, std::vector<std::tuple<int,int,int>>& orders,std::pair<int,int> fleet);
    Solution greedy_build(const Graph& graph,const std::vector<std::tuple<int,int,int>>& orders,int numDrivers,int depot);
    Solution LNS(Solution& initial,Graph& graph,double time_budget);
    void remove_order_from_driver(DriverRoute& route,const Order& order);
};

#endif