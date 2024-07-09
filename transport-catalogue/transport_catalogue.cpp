#include "transport_catalogue.h"

#include <algorithm>
#include <unordered_set>

namespace trasport_catalogue {

    void TransportCatalogue::AddBus(const std::string& bus_number, const std::vector<std::string>& bus_stops, bool is_circular) {
        all_buses_.push_back({ bus_number, bus_stops, is_circular });
        busname_to_bus_[all_buses_.back().number] = &all_buses_.back();
    
        for (const auto& bus_stop : bus_stops) {
            for (auto& stop_ : all_stops_) {
                if (stop_.name != bus_stop) {
                    continue;
                };
            
                stop_.buses.insert(bus_number);
            }
        }
    }

    void TransportCatalogue::AddStop(const std::string& stop_name,const geo::Coordinates& coordinates) {
        std::set<std::string> bus_numbers;
        for (auto& bus_ : all_buses_) {
            for (auto& stop : bus_.stops) {
                if (stop == stop_name) {
                    bus_numbers.insert(bus_.number);
                }
            }
        }
    
        all_stops_.push_back({ stop_name, coordinates, bus_numbers });
        stopname_to_stop_[all_stops_.back().name] = &all_stops_.back();
    }

    const Bus* TransportCatalogue::GetBus(const std::string_view bus_number) const {
        auto it = busname_to_bus_.find(std::string(bus_number));
        return it != busname_to_bus_.end()
            ? it->second
            : nullptr;
    }

    const Stop* TransportCatalogue::GetStop(const std::string_view stop_name) const {
        auto it = stopname_to_stop_.find(std::string(stop_name));
        return it != stopname_to_stop_.end()
            ? it->second
            : nullptr;
    }

    const RouteInfo TransportCatalogue::GetRouteInformation(const std::string& route_number) const {  
        RouteInfo route_info {};
        const Bus* bus = GetBus(route_number);
    
        bool is_circular = bus->is_circular;
        route_info.stops_count = is_circular
            ? bus->stops.size()
            : bus->stops.size() * 2 - 1;

        double route_length = 0.0;
        double geographic_length = 0.0;
        for (auto iter = bus->stops.begin(); iter + 1 != bus->stops.end(); ++iter) {
            const Stop* first_stop = stopname_to_stop_.find(*iter)->second;
            const Stop* second_stop = stopname_to_stop_.find(*(iter + 1))->second;
                 
            route_length += GetDistance(first_stop, second_stop);
            geographic_length += geo::ComputeDistance(first_stop->coordinates, second_stop->coordinates);
        }
    
        route_info.unique_stops_count = UniqueStopsCount(route_number);
        route_info.route_length = route_length;
        route_info.curvature = route_length / geographic_length;
         
        return route_info;
    }

    size_t TransportCatalogue::UniqueStopsCount(const std::string& route_number) const {
        std::unordered_set<std::string> unique_stops;
        
        for (const auto& stop : busname_to_bus_.at(route_number)->stops) {
            unique_stops.insert(stop);
        }
        
        return unique_stops.size();
    }

    const std::set<std::string> TransportCatalogue::GetBusesByStop(const std::string_view& stop_name) const {    
        return stopname_to_stop_.at(std::string(stop_name))->buses;
    }
    
    void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, const int distance) {
        route_distances_[{from, to}] = distance;
    }
    
    int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
        auto it_from_to = route_distances_.find({from, to});
        if (it_from_to != route_distances_.end()) {
            return it_from_to->second;
        }
        
        auto it_to_from = route_distances_.find({to, from});
        if (it_to_from != route_distances_.end()) {
            return it_to_from->second;
        }
        
        return 0;
    }
}