#include "tsp.hpp"
std::vector<int> Graph::get_path(int A,int B){
    std::vector<int> path;
    if(apsp_times[A][B]<0)return path;
    int cur=B;
    path.push_back(B);
    while(cur!=A){
        cur=apsp_next[A][cur];
        path.push_back(cur);
    }
    std::reverse(path.begin(),path.end());
    return path;
}

Solution TSP::greedy_build(const Graph& graph,const std::vector<std::tuple<int,int,int>>& orders,int numDrivers,int depot)
{
    std::vector<Order> order;
    order.reserve(orders.size());
    for (auto &t : orders) {
        int id,pickup,dropoff;
        std::tie(id,pickup,dropoff)=t;
        double t0=graph.apsp_times[depot][pickup];
        if (t0<0)t0=1e18; 
        order.push_back(Order(id,pickup,dropoff,t0));
    }

    //Sorting by distance from depot
    std::sort(order.begin(), order.end(),[](const Order& a, const Order& b){ return a.time_2_reach < b.time_2_reach; });
    Solution S;
    S.drivers.resize(numDrivers);
    for (int d=0;d <numDrivers;d++) {
        S.drivers[d].route.push_back(depot);
    }

    std::vector<double> max_time_till_now(numDrivers, 0.0);
    std::vector<int> lastNode(numDrivers, depot);
    //Assigning orders to drivers greedily
    for (auto &o:order) {
        double bestCost=1e18;
        int bestDriver=0;
        for (int d=0;d<numDrivers;d++) {
            if (graph.apsp_times[lastNode[d]][o.pickup]<0) continue; 
            double cost=max_time_till_now[d]+graph.apsp_times[lastNode[d]][o.pickup];
            if (cost<bestCost) {
                bestCost=cost;
                bestDriver=d;
            }
        }
        if(bestDriver==-1) {continue;}
        S.drivers[bestDriver].orders.push_back(o);
        max_time_till_now[bestDriver] = bestCost;
        lastNode[bestDriver] = o.pickup;
    }

    for (auto &DrvR:S.drivers) {
        if (DrvR.orders.empty()) continue;
        auto& route=DrvR.route;
        for (auto &o:DrvR.orders) {
            int pickup=o.pickup;
            int dropoff=o.dropoff;


            // Insert pickup by finding the best position
            double bestDelta=1e18;
            int bestPos=1; // default insertion at the start
            for (int i=0; i<(int)route.size(); i++) {
                int A=route[i];
                int B=(i+1<(int)route.size())?route[i+1]:-1;
                double AP=graph.apsp_times[A][pickup];
                if (AP<0)continue;
                double PB=(B==-1?0:graph.apsp_times[pickup][B]);
                if (PB<0)continue;
                double before=(B==-1?0 : graph.apsp_times[A][B]);
                double after=graph.apsp_times[A][pickup] + (B==-1 ? 0:graph.apsp_times[pickup][B]);
                double delta=after-before;
                if (delta<bestDelta) {
                    bestDelta=delta;
                    bestPos=i+1;
                }
            }
            route.insert(route.begin()+bestPos,pickup);


            // Inserting drop after pickup
            int pickPos=std::find(route.begin(), route.end(),pickup) - route.begin();
            bestDelta=1e18;
            int bestDropPos=pickPos+1;//default
            for (int j = pickPos + 1; j <= (int)route.size(); j++) {
                int A=route[j-1];
                int B=(j<(int)route.size()) ? route[j] : -1;
                double AD = graph.apsp_times[A][dropoff];
                if (AD<0)continue;
                double DB=(B==-1?0:graph.apsp_times[dropoff][B]);
                if (DB<0)continue;
                double before = (B == -1 ? 0 : graph.apsp_times[A][B]);
                double after  = graph.apsp_times[A][dropoff] + (B == -1 ? 0 : graph.apsp_times[dropoff][B]);
                double delta = after - before;
                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestDropPos = j;
                }
            }
            route.insert(route.begin() + bestDropPos, dropoff);
        }
    }

    double total=0;
    for (auto &DrvR:S.drivers) {
        auto &route=DrvR.route;
        auto &orders2=DrvR.orders;
        std::unordered_map<int,int> dropToOrder;
        for (auto &o:orders2) dropToOrder[o.dropoff] = o.id;
        double time=0;
        for (unsigned int i=1; i<route.size(); i++) {
            int u=route[i-1],v=route[i];
            double w=graph.apsp_times[u][v];
            if(w<0)continue;
            time+=graph.apsp_times[u][v];
            if (dropToOrder.count(v)) {
                total+=time;
            }
        }
    }
    S.total_latency=total;
    return S;
}

void TSP::remove_order_from_driver(DriverRoute& route,const Order& order){
    for(int i=0;i<(int)D.route.size();i++){
        if(D.route[i]==o.pickup || D.route[i]==o.dropoff){
            D.route.erase(D.route.begin()+i);
            i--;
        }
    }
    for(int i=0;i<(int)D.orders.size();i++){
        if(D.orders[i].id==o.id){
            D.orders.erase(D.orders.begin()+i);
            break;
        }
    }
}


Solution TSP::LNS(Solution& initial,Graph& graph,double time_budget){
    Solution best=initial;
    Solution current=initial;
    auto start_time= std::chrono::high_resolution_clock::now();
    auto present_time= std::chrono::high_resolution_clock::now();
    int K=2;
    while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-start_time).count()<time_budget){
        current=best;
        destroy_random(current,K);
        repair(current,graph);
        for(auto& D:current.drivers)two_opt(D,graph);
        current.total_latency=compute_latency(current,graph);
        if(current.total_latency<best.total_latency)best=current;
    }
    return best;
}

TSP_Result TSP::solve(Graph& graph, std::vector<std::tuple<int,int,int>>& orders,std::pair<int,int> fleet){
    Solution initial_sol=greedy_build(graph,orders,fleet.first,fleet.second);
    Solution best=initial_sol;
    TSP_Result out;
    for(int i=0;i<fleet.first;i++){
        std::vector<int> ids;
        for(auto&o:best.drivers[i].orders)ids.push_back(o.id);
        out.assignments.push_back(std::make_tuple(i,best.drivers[i].route,ids));
    }
    out.time=best.total_latency;
    return out;
};