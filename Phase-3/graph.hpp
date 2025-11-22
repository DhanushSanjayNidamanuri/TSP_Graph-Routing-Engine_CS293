#ifndef GRAPH_HPP
#define GRAPH_HPP
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <queue>
#include <algorithm>
#include <limits>
#include <cmath>
#include "tsp.hpp"
class Node{
public: 
    int id;    
    double lat, lon;
    bool is_landmark=false;
    bool isolated=false;
    // std::vector<std::string> pois;
    Node(){};
    Node(int id, double lat, double lon) : id(id), lat(lat), lon(lon) {};
};

class Edge{
public:
    int id;
    int u, v;
    double length;
    bool oneway;
    double average_time;
    Edge(){};
    Edge(int id, int u, int v, double length, bool oneway,double average_time) : id(id), u(u), v(v), length(length), oneway(oneway),average_time(average_time) {};
};
class Graph{
    std::vector<Node> node_list;
    int node_count;
    std::vector<std::vector<Edge>> adjacency_list;
    std::vector<std::vector<double>> apsp_times;
    std::vector<std::vector<double>> apsp_next;
public:
    friend class TSP;
    Graph(int node_count=0): node_count(node_count){
        adjacency_list.resize(node_count);node_list.resize(node_count);
    }
    void addNode(const Node& node);
    void addEdge(const Edge& edge);
    void preprocess();
    void dijkstra(std::vector<double>& distances,int src,,std::vector<int>& parent);
    nlohmann::json query_handler(const nlohmann::json& query);
};
#endif