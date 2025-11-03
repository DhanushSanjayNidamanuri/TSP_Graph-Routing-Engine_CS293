#include "graph.hpp"

void Graph::addNode(const Node& node) {
    node_list[node.id]=node;
}

void Graph::addEdge(const Edge& edge) {
    edge_list.insert({edge.id,edge});
    adjacency_list[edge.u][edge.id]=edge;
    if(!edge.oneway) adjacency_list[edge.v][edge.id]=edge;
}

bool Graph::removeEdge(int id) {
    Edge edge=edge_list[id];
    adjacency_list[edge.u][id].isOpen=false;
    if(!edge.oneway) adjacency_list[edge.v][id].isOpen=false;
    edge_list[id].isOpen=false;
}

bool Graph::modifyEdge(int id, double length, double average_time, std::vector<double> speed_profile) {
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

nlohmann::json Graph::query_handler(const nlohmann::json& query){
    nlohmann::json out;
    if(query.find("meta")!=query.end())return query;
    else{
        if(query["type"]=="remove_edge"){
            bool done=removeEdge(query["edge_id"]);
            out["done"]=done;
            return out;
        }
        else if(query["type"]=="modify_edge"){
            int length=edge_list[query["edge_id"]].length;
            if(query["patch"].find("length")!=query["patch"].end()){
                length=query["patch"]["length"];
            }
            std::vector<double> speed_profile=edge_list[query["edge_id"]].speed_profile;
            if(query["patch"].find("speed_profile")!=query["patch"].end()){
                length=query["patch"]["speed_profile"];
            }
            double average_time=edge_list[query["average_time"]].average_time;
            if(query["patch"].find("average_time")!=query["patch"].end()){
                length=query["patch"]["average_time"];
            }
            bool done=modifyEdge(query["edge_id"],length,average_time,speed_profile);
            out["done"]=done;
            return out;
        }
        else{
            return out;
        }
    }   
}