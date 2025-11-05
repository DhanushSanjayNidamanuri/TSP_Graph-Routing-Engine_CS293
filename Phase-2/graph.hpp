#ifndef GRAPH_HPP
#define GRAPH_HPP
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "ShortestPath.hpp"
#include "KNN.hpp"
class Node{
public: 
    int id;
    bool isValid;    
    double lat, lon;
    std::vector<std::string> pois;
    Node(int id, double lat, double lon, std::vector<std::string> pois) : id(id),isValid(1), lat(lat), lon(lon), pois(pois) {};
};

class Edge{
public:
    int id;
    int u, v;
    bool isOpen;
    double length, average_time;
    std::vector<double> speed_profile;
    bool oneway;
    std::string road_type;
    Edge(): isOpen(true){};
    Edge(int id, int u, int v, double length, double average_time, std::vector<double> speed_profile, bool oneway, std::string road_type) : id(id), u(u), v(v),isOpen(true), length(length), average_time(average_time), speed_profile(speed_profile), oneway(oneway), road_type(road_type) {};
};

class Graph{
    std::vector<Node> node_list;
    int node_count;
    std::vector<std::unordered_map<int,Edge>> adjacency_list;
    std::unordered_map<int,Edge> edge_list;

public:
    friend class ShortestPath;
    friend class KNN;
    Graph(int node_count=0): node_count(node_count){};
    void addNode(const Node& node);
    void addEdge(const Edge& edge);
    bool removeEdge(int id);
    bool modifyEdge(int id, double length, double average_time, std::vector<double> speed_profile);
    nlohmann::json query_handler(const nlohmann::json& query);
};
#endif