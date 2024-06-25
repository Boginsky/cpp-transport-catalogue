#include "stat_reader.h"
#include "input_reader.h"

#include <string>
#include <string_view>
#include <vector>
#include <iomanip>
#include <unordered_set>

namespace print_util {

    std::string GetBusNumber(const std::string& str) {
        std::string result = str.substr(4);
        return result;
    }

    std::string GetStopName(const std::string& str) {
        std::string result = str.substr(5);
        return result;
    }

    bool IsBus(const std::string& str) {
        return str.substr(0, 3) == "Bus";
    }

    void PrintBus(const std::string& entity,
                  std::ostream& output,
                  const trasport_catalogue::TransportCatalogue& tansport_catalogue) {
        std::string bus_number = GetBusNumber(entity);
        
        if (tansport_catalogue.GetBus(bus_number)) {
            auto info = tansport_catalogue.GetRouteInformation(bus_number);
            output << entity << ": " 
                << info.stops_count 
                << " stops on route, "
                << info.unique_stops_count 
                << " unique stops, " 
                << std::setprecision(6)
                << info.route_length 
                << " route length\n";
        } else {
            output << entity << ": not found\n";
        }
    }

    void PrintStop(const std::string& entity,
                   std::ostream& output,
                   const trasport_catalogue::TransportCatalogue& tansport_catalogue) {
        output << entity << ":";
        std::string stop_name = GetStopName(entity);
        
        const trasport_catalogue::Stop* stop = tansport_catalogue.GetStop(stop_name);
        if (stop == nullptr) {
            output << " not found\n";
            return;
        }
    
        const std::set<std::string>& buses = tansport_catalogue.GetBusesByStop(stop_name);
        
        if (buses.empty()) {
            output << " no buses";
        } else {
            output << " buses";
        
            for (std::string_view bus : buses) {
                output << " " << bus;
            }
        }
    
        output << "\n";
    }

    void ParseAndPrintStat(const trasport_catalogue::TransportCatalogue& tansport_catalogue,
                           std::string_view request,
                           std::ostream& output
                           ) {   
        for (const std::string_view line : input_util::Split(request, '\n')) {       
            std::string string_line = std::string(line);
            if (IsBus(string_line)) {
                PrintBus(string_line, output, tansport_catalogue);
            } else {
                PrintStop(string_line, output, tansport_catalogue);
            }     
        }   
    }   
}