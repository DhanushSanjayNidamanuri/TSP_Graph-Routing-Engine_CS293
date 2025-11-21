#include "Approx_Shortest.hpp"
int ApproxShortest::heuristic_distance(const Node& a, const Node& b) {
    double mean_lat = (a.lat + b.lat) * 0.5 *  3.14159265358979323846 / 180.0;
    double dx = (b.lon - a.lon) * 111320.0 * std::cos(mean_lat);
    double dy = (b.lat - a.lat)*111320.0;
    double ans=std::sqrt(dx * dx + dy * dy);
    return int(ans);
    }

double ApproxShortest::Hybrid_A_Star(Graph& graph,double time_limit,int source,int target,double upper_bound){
    std::priority_queue<std::tuple<double,double,int>,std::vector<std::tuple<double,double,int>>,std::greater<std::tuple<double,double,int>>> pq;
    int temp=heuristic_distance(graph.node_list[source],graph.node_list[target]);
    pq.push(std::make_tuple(temp,temp,source));
    std::vector<bool> visited(graph.node_count,false);
    int expansion=0;
    auto start_time = std::chrono::high_resolution_clock::now();
    int new_lm=-1,lm_dist=0;
    while(!pq.empty()){
        auto [dist,h_dist,u]=pq.top();pq.pop();
        expansion++;
        dist=dist-h_dist;
        if(visited[u]==true)continue;
        visited[u]=true;
        if(graph.node_list[u].is_landmark){new_lm=u;lm_dist=dist;}
        if(u==target){
            return dist;
        }
        if(expansion%512==0){
            auto present_time= std::chrono::high_resolution_clock::now();
            if((std::chrono::duration_cast<std::chrono::milliseconds>(present_time - start_time).count()>(time_limit*19)/20)){
                auto [u_lm,u_lm_dist]=graph.nearest_into_landmark[u];
                auto [dest_lm,dest_lm_dist]=graph.nearest_outOf_landmark[target];
                double lm_to_lm=graph.landmark_to_landmark[u_lm][dest_lm];
                double top_dist=u_lm_dist+dest_lm_dist+lm_to_lm;
                double top_lm_dist=lm_dist+graph.landmark_to_landmark[new_lm][dest_lm]+dest_lm_dist;
                if (lm_to_lm<0 || u_lm_dist<0 || dest_lm_dist<0){
                    top_dist=std::numeric_limits<double>::max();
                }
                if(graph.landmark_to_landmark[new_lm][dest_lm]<0 || dest_lm_dist<0)top_lm_dist=std::numeric_limits<double>::max();
                if(upper_bound<0)upper_bound=std::numeric_limits<double>::max();
                double out=std::min(top_dist,top_lm_dist);
                out=std::min(out,upper_bound);
                if(out==std::numeric_limits<double>::max())return -1;
                return out;
            }
        }
        for(auto e:graph.adjacency_list[u]){
            int v =(e.u==u) ? e.v : e.u;
            if(!visited[v] && ((!e.oneway) || (e.u == u))){
                double temp=heuristic_distance(graph.node_list[v],graph.node_list[target]);
                pq.push(std::make_tuple(dist+e.length+temp,temp,v));
            }
        }
    }
    return upper_bound;
}


ApproxShortest_Result ApproxShortest::findApprox(Graph& graph, int id, std::vector<std::pair<int,int>>& queries, double time_budget, double max_error){
    double tuning_factor=(180/max_error) +1;
    std::vector<std::tuple<int,int,double>> dists;
    auto total_time=0;
    int total_queries=queries.size();
    for(int i=0;i<total_queries;i++){
        auto [source,target]=queries[i];
        auto start_time = std::chrono::high_resolution_clock::now();
        auto [src_lm,src_lm_dist]=graph.nearest_into_landmark[source];
        auto [dest_lm,dest_lm_dist]=graph.nearest_outOf_landmark[target];
        double lm_to_lm=graph.landmark_to_landmark[src_lm][dest_lm];

        double avg_time_left=(time_budget-total_time)/(total_queries-i);
        if(source==target || (graph.node_list[source].isolated) || (graph.node_list[target].isolated) ){
            dists.push_back(std::make_tuple(source,target,-1*!(source==target)));
        }
        else if(src_lm_dist<0 || lm_to_lm<0 || dest_lm_dist<0){
            double temp=Hybrid_A_Star(graph,avg_time_left,source,target,-1);
            dists.push_back(std::make_tuple(source,target,temp));
        }
        else if((src_lm_dist+dest_lm_dist)==0){
            dists.push_back(std::make_tuple(source,target,lm_to_lm+src_lm_dist+dest_lm_dist));
        }
        else if(((lm_to_lm)/(src_lm_dist+dest_lm_dist))>=tuning_factor || avg_time_left<=(total_time/2*queries.size())){
           dists.push_back(std::make_tuple(source,target,lm_to_lm+src_lm_dist+dest_lm_dist));
        }
        else{
            double temp=Hybrid_A_Star(graph,avg_time_left,source,target,lm_to_lm+src_lm_dist+dest_lm_dist);
            dists.push_back(std::make_tuple(source,target,temp));
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        total_time+=std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    }

    ApproxShortest_Result out(id,dists);
    return out;
};

