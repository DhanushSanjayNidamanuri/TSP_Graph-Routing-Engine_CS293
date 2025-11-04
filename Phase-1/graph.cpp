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
    if(edge_list.find(id)==edge_list.end()) return false;
    Edge edge=edge_list[id];
    adjacency_list[edge.u][id].isOpen=false;
    if(!edge.oneway) adjacency_list[edge.v][id].isOpen=false;
    edge_list[id].isOpen=false;
    return true;
}

bool Graph::modifyEdge(int id, double length, double average_time, std::vector<double> speed_profile) {
    if(edge_list.find(id)==edge_list.end()) return false;
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
    return true;
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
        else if(query["type"]=="shortest_path"){
            ShortestPath temp;
            std::vector<int> forbidden_nodes=query["constraints"]["forbidden_nodes"];
            std::vector<std::string> forbidden_types=query["constraints"]["forbidden_types"];
            ShortestPath_Result tempout=temp.findShortestPath(*this,query["id"],query["source"],query["target"],query["mode"],forbidden_nodes,forbidden_types);
            out["id"]=tempout.id;
            out["possible"]=tempout.possible;
            out["minimum_time/minimum_distance"]=tempout.min_dist_or_time;
            out["path"]=tempout.path;
            return out;
        }
        else if(query["type"]=="knn"){
            KNN temp;
            Result_KNN tempout=temp.findKNN(*this,query["id"],query["query_point"]["lat"],query["query_point"]["lon"],query["poi"],query["k"],query["metric"]);
            out["id"]=tempout.id;
            out["nodes"]=tempout.node_ids;
            return out;
        }
        else{
            return out;
        }
    }   
}