#include "tsp.hpp"

Solution TSP::greedy_build(const Graph& graph,const std::vector<std::tuple<int,int,int>>& orders,int numDrivers,int depot)
{
    std::vector<Order> order;
    order.reserve(orders.size());
    for (auto &t : orders) {
        int id,pickup,dropoff;
        std::tie(id,pickup,dropoff)=t;
        double t0=graph.apsp_times[depot][pickup];
        order.push_back(Order(id,pickup,dropoff,t0));
    }

    // 1. Sort by distance from depot (Minimum Latency principle)
    std::sort(order.begin(), order.end(),[](const Order& a, const Order& b){ return a.time_2_reach < b.time_2_reach; });

    Solution S;
    S.drivers.resize(numDrivers);
    for (int d=0;d <numDrivers;d++) {
        S.drivers[d].route.push_back(depot); // start with depot
    }

    std::vector<double> max_time_till_now(numDrivers, 0.0);
    std::vector<int> lastNode(numDrivers, depot);

    //Assigning orders to drivers greedily
    for (auto &o:order) {
        double bestCost=1e18;
        int bestDriver=0;
        for (int d=0;d<numDrivers;d++) {
            double cost=max_time_till_now[d]+graph.apsp_times[lastNode[d]][o.pickup];
            if (cost<bestCost) {
                bestCost=cost;
                bestDriver=d;
            }
        }
        // assign
        S.drivers[bestDriver].orders.push_back(o);
        max_time_till_now[bestDriver] = bestCost;
        lastNode[bestDriver] = o.pickup;
    }

    // 3. Build routes using cheapest insertion (pickup first, then drop)
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
                int A = route[j-1];
                int B = (j < (int)route.size()) ? route[j] : -1;
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

    //Computing total latency (sum of all orders completion times)
    double total=0;

    for (auto &DrvR:S.drivers) {
        auto &route=DrvR.route;
        auto &orders2=DrvR.orders;

        // mapping all the dropping points
        std::unordered_map<int,int> dropToOrder;
        for (auto &o:orders2) dropToOrder[o.dropoff] = o.id;

        double time=0;
        for (unsigned int i=1; i<route.size(); i++) {
            int u=route[i-1],v=route[i];
            time+=graph.apsp_times[u][v];
            if (dropToOrder.count(v)) {
                total+=time;//order delivered time
            }
        }
    }
    S.total_latency=total;
    return S;
}
TSP_Result TSP::solve(Graph& graph, std::vector<std::tuple<int,int,int>>& orders,std::pair<int,int> fleet){
    Solution initial_sol=greedy_build(graph,orders,fleet.first,fleet.second);
    //TEMP
    TSP_Result out;
    for(int i=0;i<fleet.first;i++){
        std::vector<int> order_ids;
        for(auto& o:initial_sol.drivers[i].orders){
            order_ids.push_back(o.id);
        }
        out.assignments.push_back(std::make_tuple(i,initial_sol.drivers[i].route,order_ids));
    }
    out.time=initial_sol.total_latency;

    return out;
    //
    //AFTER THIS WE WILL DO LNS TO IMPROVE THE GREEDY SOLUTION
};