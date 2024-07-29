#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>
#include <unordered_map>

namespace trasport_catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
        std::set<std::string> buses;
    };

    struct Bus {
        std::string number;
        std::vector<const Stop*> stops;
        bool is_circular;
    };

    struct RouteInfo {
        size_t stops_count;
        size_t unique_stops_count;
        double route_length;
        double curvature;
    };
}