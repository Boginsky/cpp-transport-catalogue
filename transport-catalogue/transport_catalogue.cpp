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
        std::unordered_set<std::string> bus_numbers;
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

    const Bus* TransportCatalogue::GetBus(const std::string_view& bus_number) const {
        return busname_to_bus_.count(std::string(bus_number)) 
            ? busname_to_bus_.at(std::string(bus_number)) 
            : nullptr;
    }

    const Stop* TransportCatalogue::GetStop(const std::string_view& stop_name) const {
        return stopname_to_stop_.count(std::string(stop_name)) 
            ? stopname_to_stop_.at(std::string(stop_name)) 
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
        for (auto iter = bus->stops.begin(); iter + 1 != bus->stops.end(); ++iter) {
            geo::Coordinates first_stop_coordinates = stopname_to_stop_.find(*iter)->second->coordinates;
            geo::Coordinates second_stop_cooridnates = stopname_to_stop_.find(*(iter + 1))->second->coordinates;
     
            double distance = geo::ComputeDistance(first_stop_coordinates, second_stop_cooridnates);
            
            route_length += distance;
        }
    
        route_info.unique_stops_count = UniqueStopsCount(route_number);
        route_info.route_length = route_length;

        return route_info;
    }

    size_t TransportCatalogue::UniqueStopsCount(const std::string& route_number) const {
        std::unordered_set<std::string> unique_stops;
        
        for (const auto& stop : busname_to_bus_.at(route_number)->stops) {
            unique_stops.insert(stop);
        }
        
        return unique_stops.size();
    }

    const std::vector<std::string_view> TransportCatalogue::GetBusesByStop(const std::string_view& stop_name) const {    
        const auto& buses = stopname_to_stop_.at(std::string(stop_name))->buses;
        std::vector<std::string_view> result(buses.begin(), buses.end());
        std::sort(result.begin(), result.end());

        return result;
    }
}
