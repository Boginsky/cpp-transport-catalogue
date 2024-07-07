#pragma once

#include "geo.h"

#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <set>

namespace trasport_catalogue {
   
    struct Bus {
        std::string number;
        std::vector<std::string> stops;
        bool is_circular;
    };
    
    struct Stop {
        std::string name;
        geo::Coordinates coordinates;
        std::set<std::string> buses;
    };
     
    struct RouteInfo {
        size_t stops_count;
        size_t unique_stops_count;
        double route_length;
        double curvature;
    };

    class TransportCatalogue {
        public:
            const std::set<std::string> GetBusesByStop(const std::string_view& stop_name) const;

            void AddBus(const std::string& bus_number, const std::vector<std::string>& bus_stops, bool is_circular);
            void AddStop(const std::string& stop_name, const geo::Coordinates& coordinates);
        
            void SetDistance(const Stop* from, const Stop* to, const int distance);
            int GetDistance(const Stop* from, const Stop* to) const;
    
            const Bus* GetBus(const std::string_view bus_number) const;
            const Stop* GetStop(const std::string_view stop_name) const;
            const RouteInfo GetRouteInformation(const std::string& route_number) const;
        private:
            struct RouteDistanceHasher {
     size_t operator()(const std::pair<const Stop*, const Stop*>& points) const {
            size_t hash_first = std::hash<const void*>{}(points.first);
            size_t hash_second = std::hash<const void*>{}(points.second);
            return hash_first + hash_second * 37;
        }
            };
            
            std::deque<Bus> all_buses_;
            std::deque<Stop> all_stops_;
    
            std::unordered_map<std::string, const Bus*> busname_to_bus_;
            std::unordered_map<std::string,  const Stop*> stopname_to_stop_;
        
            size_t UniqueStopsCount(const std::string& route_number) const;
        
            std::unordered_map<std::pair<const Stop*, const Stop*>, int, RouteDistanceHasher> route_distances_; 
    };
}