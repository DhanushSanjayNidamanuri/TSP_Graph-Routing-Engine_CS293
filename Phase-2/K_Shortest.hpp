#ifndef KSHORTEST_HPP
#define KSHORTEST_HPP

#include "graph.hpp"
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <algorithm>
class Graph;
class Node;
class Edge;

class KShortestPaths_Result {
public:
    int id;
    std::vector<std::pair<std::vector<int>,int>> paths;
    KShortestPaths_Result(int id, std::vector<std::pair<std::vector<int>,int>> paths) : id(id), paths(paths) {};
};

class KShortestPaths {
public:
    KShortestPaths_Result findShortest(Graph& graph,std::string type, int id, int source, int target, int k, std::string mode="distance", int overlap_threshold=0);
    std::vector<std::pair<std::vector<int>,int>> KShortest(Graph& graph, int source, int target, int k, std::string mode);
    std::vector<std::pair<std::vector<int>,int>> KShortest_heuristic(Graph& graph, int source, int target, int k, int overlap_threshold);
};

#endif