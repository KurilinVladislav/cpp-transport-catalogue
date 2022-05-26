#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

int main() {

    transport::input::InputReader reader;
    reader.Read(std::cin);

    transport::TransportCatalogue catalogue;
    catalogue.FillInfo(reader);

    transport::stat::StatReader stat_reader;
    stat_reader.Read(std::cin);
    
    stat_reader.ProcessQueries(catalogue);

    return 0;
}
