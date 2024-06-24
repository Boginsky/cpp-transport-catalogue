#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    trasport_catalogue::TransportCatalogue catalogue;
    
    input_util::InputReader reader;
    reader.ReadInfo(catalogue);
    
    int stat_request_count;
    cin >> stat_request_count >> ws;
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(cin, line);
        print_util::ParseAndPrintStat(catalogue, line, cout);
    }
}