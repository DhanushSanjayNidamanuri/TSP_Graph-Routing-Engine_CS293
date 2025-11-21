#ifndef KSHORTEST_HPP
#define KSHORTEST_HPP

#include "graph.hpp"
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <algorithm>
#include <unordered_set>

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
    std::vector<std::pair<std::vector<int>,int>> KShortest(Graph& graph, int source, int target,unsigned int k, std::string mode);
    std::vector<std::pair<std::vector<int>,int>> KShortest_heuristic(Graph& graph, int source, int target,unsigned int k, int overlap_threshold);
};

std::pair<std::vector<int>, double> AstarShortestPath(Graph& graph, int source, int target, std::string mode, const std::vector<std::pair<int, int>>& removedEdges={});
double path_distance(Graph& graph, std::vector<int>& path);
bool is_path(const std::vector<int>& path);
bool samePaths(const std::vector<int>& path1, const std::vector<int>& path2);

#endif