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

#include "K_Shortest.hpp"
#include "Approx_Shortest.hpp"

class Node{
public: 
    int id;    
    double lat, lon;
    std::vector<std::string> pois;
    Node(int id, double lat, double lon, std::vector<std::string> pois) : id(id), lat(lat), lon(lon), pois(pois) {};
};

class Edge{
public:
    int id;
    int u, v;
    double length, average_time;
    std::vector<double> speed_profile;
    std::string road_type;
    Edge(){};
    Edge(int id, int u, int v, double length, double average_time, std::vector<double> speed_profile, bool oneway, std::string road_type) : id(id), u(u), v(v), length(length), average_time(average_time), speed_profile(speed_profile), road_type(road_type) {};
};
class Shortcut_Edge{
public:
    int v;
    double length;
    int hidden_node;
    bool is_Shortcut;
    
};
class Graph{
    std::vector<Node> node_list;
    int node_count;
    std::vector<std::vector<Edge>> adjacency_list;
    //preprocess for approximate paths
    std::vector<int> rank;
    std::vector<std::vector<Shortcut_Edge>> upward_edges;
    std::vector<std::vector<Shortcut_Edge>> downward_edges;

public:
    friend class KShortestPaths;
    friend class ApproxShortest_Result;

    Graph(int node_count=0): node_count(node_count){};
    void addNode(const Node& node);
    void addEdge(const Edge& edge);
    bool removeEdge(int id);
    bool modifyEdge(int id, double length, double average_time, std::vector<double> speed_profile);
    void preprocess(int witness_limit=40);
    nlohmann::json query_handler(const nlohmann::json& query);
};
#endif