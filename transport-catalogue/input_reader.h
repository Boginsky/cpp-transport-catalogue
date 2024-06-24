#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <string>
#include <string_view>
#include <vector>

namespace input_util {

    geo::Coordinates ParseCoordinates(std::string_view str);
    std::vector<std::string_view> Split(std::string_view string, char delim);
    std::string_view Trim(std::string_view string);
    std::vector<std::string_view> ParseRoute(std::string_view route);

    struct CommandDescription {
        explicit operator bool() const {
            return !command.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string command;      
        std::string id;           
        std::string description;  
    };

    class InputReader {
    public:
        void ParseLine(std::string_view line);
        void ApplyCommands(trasport_catalogue::TransportCatalogue& catalogue) const;
        void ReadInfo(trasport_catalogue::TransportCatalogue& catalogue);
    private:
        std::vector<CommandDescription> commands_;
    };
}