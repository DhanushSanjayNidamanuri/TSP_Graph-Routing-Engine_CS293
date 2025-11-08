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
    // std::vector<std::string> pois;
    Node(int id, double lat, double lon) : id(id), lat(lat), lon(lon) {};
};

class Edge{
public:
    int id;
    int u, v;
    double length;
    // double average_time;
    bool oneway;
    // std::vector<double> speed_profile;
    // std::string road_type;
    Edge(){};
    Edge(int id, int u, int v, double length, bool oneway) : id(id), u(u), v(v), length(length), oneway(oneway) {};
};
class Shortcut_Edge{
    //all of these are one ways only
public:
    int v;
    double length;
    int hidden_node;
    bool is_Shortcut;
    Shortcut_Edge(int v,double length,int hidden_node,bool is_Shortcut=true):v(v),length(length),hidden_node(hidden_node),is_Shortcut(is_Shortcut){};

    
};
class Graph{
    std::vector<Node> node_list;
    int node_count;
    std::vector<std::vector<Edge>> adjacency_list;
    //preprocess for approximate paths
    std::vector<int> rank;
    std::vector<std::unordered_map<int,Shortcut_Edge>> processed_outgoing_edges;
    std::vector<std::unordered_map<int,Shortcut_Edge>> processed_incoming_edges;
    std::vector<std::vector<Shortcut_Edge>> upward_edges; //outgoing 
    std::vector<std::vector<Shortcut_Edge>> downward_edges; //incoming

public:
    friend class KShortestPaths;
    friend class ApproxShortest_Result;

    Graph(int node_count=0): node_count(node_count){adjacency_list.resize(node_count);node_list.resize(node_count);processed_incoming_edges.resize(node_count);processed_outgoing_edges.resize(node_count);}
    void addNode(const Node& node);
    void addEdge(const Edge& edge);
    // bool removeEdge(int id);
    // bool modifyEdge(int id, double length, double average_time, std::vector<double> speed_profile);
    void preprocess(int witness_limit=40);
    double witness_search(int source,int target,int avoid,double dist_limit,int algo_limit=40);
    void make_processed_adjacency_list();
    nlohmann::json query_handler(const nlohmann::json& query);
};
#endif