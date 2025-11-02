#include "graph.hpp"

void Graph::addNode(const Node& node) {
    node_list[node.id]=node;
}

void Graph::addEdge(const Edge& edge) {
    edge_list.insert({edge.id,edge});
    adjacency_list[edge.u][edge.id]=edge;
    if(!edge.oneway) adjacency_list[edge.v][edge.id]=edge;
}

void Graph::removeEdge(int id) {
    Edge edge=edge_list[id];
    adjacency_list[edge.u][id].isOpen=false;
    if(!edge.oneway) adjacency_list[edge.v][id].isOpen=false;
    edge_list[id].isOpen=false;
}

void Graph::modifyEdge(int id, double length, double average_time, std::vector<double> speed_profile) {
    if(edge_list.find(id)==edge_list.end()) return;
    Edge edge=edge_list[id];
    edge.isOpen=true;
    edge.length=length;
    edge.average_time=average_time;
    edge.speed_profile.resize(speed_profile.size());
    for(int i=0;i<speed_profile.size();i++) {
        edge.speed_profile[i]=speed_profile[i];
    }
    edge_list[id]=edge;
    adjacency_list[edge.u][edge.id]=edge;
    if(!edge.oneway) adjacency_list[edge.v][edge.id]=edge;
}