#ifndef GRAPH_HPP
#define GRAPH_HPP
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

class Node{
public:
    int id;
    bool isValid;
    double lat, lon;
    std::vector<std::string> pois;
    Node(int id, double lat, double lon, std::vector<std::string> pois) : id(id), isValid(1), lat(lat), lon(lon), pois(pois) {};
};

class Edge{
public:
    int id;
    int u, v;
    double length, average_time;
    std::vector<double> speed_profile;
    bool oneway;
    std::string road_type;
    Edge(int id, int u, int v, double length, double average_time, std::vector<double> speed_profile, bool oneway, std::string road_type) : id(id), u(u), v(v), length(length), average_time(average_time), speed_profile(speed_profile), oneway(oneway), road_type(road_type) {};
};

class Graph{
    std::unordered_map<int,Node> node_list;
    std::unordered_map<int,std::unordered_map<int,std::unordered_map<int,Edge>>> adjacency_list;
    std::unordered_map<int,Edge> edge_list;

public:
    friend class ShortestPath;
    void addNode(const Node& node);
    void addEdge(const Edge& edge);
    Node removeNode(int id);
    Edge removeEdge(int id);
    Edge modifyEdge(int id, double length);
};
#endif