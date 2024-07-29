#include "json_reader.h"

int main() {
    trasport_catalogue::TransportCatalogue catalogue;
    JsonReader json_doc(std::cin, catalogue);
    
    const auto& stat_requests = json_doc.GetStatRequests();
    json_doc.ProcessRequests(stat_requests);   
}