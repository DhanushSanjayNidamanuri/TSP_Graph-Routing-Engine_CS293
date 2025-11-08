#include "graph.hpp"
class K_Exact;
class K_Heuristic;
class Approx_Shortest;
void Graph::addNode(const Node& node) {
    node_list.push_back(node);
    node_count++;
    adjacency_list.push_back(std::unordered_map<int,Edge> ());
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
    edge.speed_profile=speed_profile;
    edge_list[id]=edge;
    adjacency_list[edge.u][edge.id]=edge;
    if(!edge.oneway) adjacency_list[edge.v][edge.id]=edge;
    return true;
}

nlohmann::json Graph::query_handler(const nlohmann::json& query){
    nlohmann::json out;
    if(query["type"]=="k_shortest_paths" || query["type"]=="k_shortest_paths_heuristic"){
        KShortestPaths temp;
        KShortestPaths_Result tempout=temp.findShortest(*this,query["type"],query["id"],query["source"],query["target"],query["mode"],query["k"],query["mode"]);
        out["id"]=tempout.id;
        std::vector<nlohmann::json> tempdists;
        for(auto [x,y]:tempout.paths){
            nlohmann::json inner_json;
            inner_json["path"]=x;
            inner_json["length"]=y;
            tempdists.push_back(inner_json);
        }
        out["paths"]=tempdists;
        return out;
    }
    else if(query["type"]=="approx_shortest_path"){
        ApproxShortest temp;
        std::vector<std::pair<int,int>> queries_temp;
        for(auto x:query["queries"]){
            queries_temp.push_back(std::make_pair(x["source"],x["target"]));
        }
        ApproxShortest_Result tempout=temp.findApprox(*this,query["id"],queries_temp,query["time_budget_ms"],query["acceptable_error_pct"]);
        out["id"]=tempout.id;
        std::vector<nlohmann::json> tempdists;
        for(auto [x,y,z]:tempout.distances){
            nlohmann::json inner_json;
            inner_json["source"]=x;
            inner_json["target"]=y;
            inner_json["approx_shortest_distance"]=z;
            tempdists.push_back(inner_json);
        }
        out["distances"]=tempdists;
        return out;
    }
    else{
        return out;
    }
    
}