#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>
#include <string>
#include <map>
#include <tuple>

namespace transport {
    struct Settings {
        double bus_velocity_{0};
        int bus_wait_time_{0};    
    };
    
    struct GraphInfo {
        std::string name;
        size_t quality;
        double weight;
    };
    
    struct Info {
           std::optional<graph::Router<double>::RouteInfo> route_info;
           std::map<size_t, GraphInfo> grapth_info_by_edge_id;
    };
    
    class Router {
        public:
            Router() = default;
    
            Router(const Catalogue& catalogue, Settings settings) {
                settings_ = settings;
                BuildGraph(catalogue);
            }
    
            const Info FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;

        private:        
            Settings settings_;
       
            graph::DirectedWeightedGraph<double> graph_;
            std::map<std::string, graph::VertexId> stop_ids_;
            std::unique_ptr<graph::Router<double>> router_;
        
            const graph::DirectedWeightedGraph<double>& BuildGraph(const Catalogue& catalogue);
            const graph::DirectedWeightedGraph<double>& GetGraph() const;
    };
}