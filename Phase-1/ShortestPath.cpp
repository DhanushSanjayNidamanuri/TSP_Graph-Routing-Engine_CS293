#include<ShortestPath.hpp>
ShoretestPath_Result ShortestPath::findShortestPath(Graph& graph, int id, int source, int target,
     const std::string& mode, const std::vector<int>& forbidden_nodes,
        const std::vector<std::string>& forbidden_road_types){
            for(auto fb_id:forbidden_nodes){
                graph.node_list[fb_id].isValid=false;
            }
            if(mode=="time"){
                int node_count=graph.node_list.size();
                std::priority_queue<std::pair<int,int>> pq;
                pq.push(std::make_pair(0,source));
                std::vector<bool> visited(node_count,false);
                while(!pq.empty()){
                    auto [neg_time,u]=pq.top();pq.pop();
                    if(visited[u]==true)continue;
                    visited[u]=true;
                    for(auto p:graph.adjacency_list[u]){
                        if(visited[p.second.v]==false && graph.node_list[p.second.v].isValid==true){
                            
                        }
                    }
                }
            }
            else if(mode=="distance"){
                
            }

        }