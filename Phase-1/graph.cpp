#include "graph.hpp"

void Graph::addNode(const Node& node) {
    node_list[node.id]=node;
}

void Graph::addEdge(const Edge& edge) {
    edge_list.insert({edge.id,edge});
    adjacency_list[edge.u][edge.id]=edge;
    if(!edge.oneway) adjacency_list[edge.v][edge.id]=edge;
}

Edge Graph::removeEdge(int id) {
    Edge edge=edge_list[id];
    adjacency_list[edge.u].erase(edge.id);
    if(!edge.oneway) adjacency_list[edge.v].erase(edge.id);
    edge_list.erase(id);
    return edge;
}

Edge Graph::modifyEdge(int id, double length) {
    Edge& edge=edge_list[id];
    edge.length=length;
    adjacency_list[edge.u][edge.id].length=length;
    if(!edge.oneway) adjacency_list[edge.v][edge.id].length=length;
    return edge_list[id];
}