#include <iostream>
#include <string>

#include "input_reader.h"

using namespace std;

int main() {
    trasport_catalogue::TransportCatalogue catalogue;
    input_util::InputReader reader;
    
    reader.ReadInfo(cin, catalogue);
    reader.OutputInfo(cin, catalogue, cout);
}