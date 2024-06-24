#pragma once

#include "transport_catalogue.h"

#include <string_view>

namespace print_util {
    
    void ParseAndPrintStat(const trasport_catalogue::TransportCatalogue& tansport_catalogue,
                           std::string_view request,
                           std::ostream& output);
}
