#include "json_reader.h"

int main() {
    trasport_catalogue::TransportCatalogue catalogue;
    JsonReader json_doc(std::cin, catalogue);
    json_doc.ProcessRequests();   
}