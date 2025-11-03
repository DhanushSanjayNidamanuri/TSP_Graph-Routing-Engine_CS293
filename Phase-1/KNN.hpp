#ifndef KNN_HPP
#define KNN_HPP

#include "graph.hpp"
#include <vector>
#include <queue>
#include <string>

class Result_KNN {
public:
    int id;
    std::vector<int> node_ids;
    Result_KNN(int id, std::vector<int> node_ids) : id(id), node_ids(node_ids) {};
};

class KNN {
public:
    Result_KNN findKNN(const Graph& graph, int id, double lat, double lon, const std::string& poi, int k, const std::string& metric);

   
    std::vector<int> findKNN_Euclidean(const Graph& graph, double lat, double lon, const std::string& poi, int k);
    
      
    std::vector<int> findKNN_ShortestPath(const Graph& graph, double lat, double lon, const std::string& poi, int k);
};

#endif