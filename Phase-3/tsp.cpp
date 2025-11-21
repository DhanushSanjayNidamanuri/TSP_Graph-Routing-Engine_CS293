#include "tsp.hpp"

Solution TSP::greedy_build(const Graph& graph,const std::vector<std::tuple<int,int,int>>& orders,int numDrivers,int depot)
{
    // Convert orders
    std::vector<Order> ord;
    ord.reserve(orders.size());

    for (auto &t : orders) {
        int oid, pu, dr;
        std::tie(oid, pu, dr) = t;
        double d0 = graph.apsp_times[depot][pu];
        ord.push_back({oid, pu, dr, d0});
    }

    // 1. Sort by distance from depot (Minimum Latency principle)
    std::sort(ord.begin(), ord.end(),
              [](const Order& a, const Order& b){ return a.depDist < b.depDist; });

    // Create drivers
    Solution S;
    S.drivers.resize(numDrivers);
    for (int d = 0; d < numDrivers; d++) {
        S.drivers[d].route.push_back(depot); // start with depot
    }

    std::vector<double> load(numDrivers, 0.0);
    std::vector<int> lastNode(numDrivers, depot);

    // 2. Assign orders to drivers greedily
    for (auto &o : ord) {

        double bestCost = 1e18;
        int bestDriver = 0;

        for (int d = 0; d < numDrivers; d++) {
            double cost = load[d] + graph.apsp_times[lastNode[d]][o.p];
            if (cost < bestCost) {
                bestCost = cost;
                bestDriver = d;
            }
        }

        // assign
        S.drivers[bestDriver].orders.push_back(o);
        load[bestDriver] = bestCost;
        lastNode[bestDriver] = o.p;
    }

    // 3. Build routes using cheapest insertion (pickup first, then drop)
    for (auto &DRV : S.drivers) {

        // skip if no orders
        if (DRV.orders.empty()) continue;

        auto& route = DRV.route;

        for (auto &o : DRV.orders) {

            int p = o.p;
            int d = o.d;

            // Insert pickup: find best position
            double bestDelta = 1e18;
            int bestPos = 1; // default insertion at end

            for (int i = 0; i < route.size(); i++) {
                int A = route[i];
                int B = (i+1 < route.size()) ? route[i+1] : -1;

                double before = (B == -1 ? 0 : graph.apsp_times[A][B]);
                double after  = graph.apsp_times[A][p] + (B == -1 ? 0 : graph.apsp_times[p][B]);
                double delta = after - before;

                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestPos = i+1;
                }
            }
            route.insert(route.begin() + bestPos, p);

            // Insert drop: must be after pickup
            int pickPos = std::find(route.begin(), route.end(), p) - route.begin();
            bestDelta = 1e18;
            int bestDropPos = pickPos + 1;

            for (int j = pickPos + 1; j <= route.size(); j++) {
                int A = route[j-1];
                int B = (j < route.size()) ? route[j] : -1;

                double before = (B == -1 ? 0 : graph.apsp_times[A][B]);
                double after  = graph.apsp_times[A][d] + (B == -1 ? 0 : graph.apsp_times[d][B]);
                double delta = after - before;

                if (delta < bestDelta) {
                    bestDelta = delta;
                    bestDropPos = j;
                }
            }

            route.insert(route.begin() + bestDropPos, d);
        }
    }

    // 4. Compute total latency (sum of drop completion times)
    double total = 0;

    for (auto &DRV : S.drivers) {
        auto &route = DRV.route;
        auto &orders2 = DRV.orders;

        // map drop → order id
        std::unordered_map<int,int> dropToOrder;
        for (auto &o : orders2) dropToOrder[o.d] = o.id;

        double time = 0;
        for (int i = 1; i < route.size(); i++) {
            int u = route[i-1], v = route[i];
            time += graph.apsp_times[u][v];

            if (dropToOrder.count(v)) {
                total += time;  // completion time
            }
        }
    }

    S.total_latency = total;
    return S;
}

TSP_RESULT TSP::solve(Graph& graph, std::vector<std::tuple<int,int,int>>& orders,std::pair<int,int> fleet){
    Solution initial_sol=greedy_build(graph,orders,fleet.first,fleet.second);
    //AFTER THIS WE WILL DO LNS TO IMPROVE THE GREEDY SOLUTION
};