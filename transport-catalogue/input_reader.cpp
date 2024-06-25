#include "input_reader.h"

#include <iostream>

namespace input_util {
    
    /**
     * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
     */
    geo::Coordinates ParseCoordinates(std::string_view str) {
        static const double nan = std::nan("");

        auto not_space = str.find_first_not_of(' ');
        auto comma = str.find(',');

        if (comma == str.npos) {
            return {nan, nan};
        }

        auto not_space2 = str.find_first_not_of(' ', comma + 1);

        double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
        double lng = std::stod(std::string(str.substr(not_space2)));

        return {lat, lng};
    }

    /**
     * Удаляет пробелы в начале и конце строки
     */
    std::string_view Trim(std::string_view string) {
        const auto start = string.find_first_not_of(' ');
        if (start == string.npos) {
            return {};
        }
        return string.substr(start, string.find_last_not_of(' ') + 1 - start);
    }

    /**
     * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
     */
    std::vector<std::string_view> Split(std::string_view string, char delim) {
        std::vector<std::string_view> result;

        size_t pos = 0;
        while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
            auto delim_pos = string.find(delim, pos);
            if (delim_pos == string.npos) {
                delim_pos = string.size();
            }
            if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                result.push_back(substr);
            }
            pos = delim_pos + 1;
        }

        return result;
    }

    /**
     * Парсит маршрут.
     * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
     * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
     */
    std::vector<std::string_view> ParseRoute(std::string_view route) {
        if (route.find('>') != route.npos) {
            return Split(route, '>');
        }

        auto stops = Split(route, '-');
        std::vector<std::string_view> results(stops.begin(), stops.end());
        results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

        return results;
    }

    CommandDescription ParseCommandDescription(std::string_view line) {
        auto colon_pos = line.find(':');
        if (colon_pos == line.npos) {
            return {};
        }

        auto space_pos = line.find(' ');
        if (space_pos >= colon_pos) {
            return {};
        }

        auto not_space = line.find_first_not_of(' ', space_pos);
        if (not_space >= colon_pos) {
            return {};
        }

        return {std::string(line.substr(0, space_pos)),
                std::string(line.substr(not_space, colon_pos - not_space)),
                std::string(line.substr(colon_pos + 1))};
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = ParseCommandDescription(line);

        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    inline static const std::string BUS_COMMAND_NAME = "Bus";

    void InputReader::ApplyCommands(trasport_catalogue::TransportCatalogue& catalogue) const {
        for (const CommandDescription& command_ : commands_) {
            if (command_.command == BUS_COMMAND_NAME) {
                std::vector<std::string> bus_stops;
                for (const auto& elem : ParseRoute(command_.description)) {
                    bus_stops.emplace_back(elem);
                }
            
                std::string& first_element = bus_stops.front();
                std::string& last_element = bus_stops.back();
                bool is_circle = first_element == last_element;                          
            
                catalogue.AddBus(command_.id, bus_stops, is_circle);
            } else {
                geo::Coordinates coordinates = ParseCoordinates(command_.description);
                catalogue.AddStop(command_.id, coordinates);
            }
        }
    }
    
    void InputReader::ReadInfo(std::istream& input_stream, trasport_catalogue::TransportCatalogue& catalogue) {
        int base_request_count;
        input_stream >> base_request_count >> std::ws;

        for (int i = 0; i < base_request_count; ++i) {
            std::string line;
            std::getline(input_stream, line);
            ParseLine(line);
        }

        ApplyCommands(catalogue);
    }
    
    void InputReader::OutputInfo(std::istream& input_stream, trasport_catalogue::TransportCatalogue& catalogue, std::ostream& output_stream) {      
        int stat_request_count;
        input_stream >> stat_request_count >> std::ws;
        
        for (int i = 0; i < stat_request_count; ++i) {
            std::string line;
            std::getline(input_stream, line);
            print_util::ParseAndPrintStat(catalogue, line, output_stream);
        }
    }
}