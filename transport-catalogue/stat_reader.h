#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "transport_catalogue.h"

namespace transport {
    
namespace stat {

class StatReader {
public:
    StatReader() {}

    void Read(std::istream& input);

    void ProcessQueries(const TransportCatalogue& catalogue, std::ostream& out = std::cout) const;

private:
    enum QueryType {
        BUS,
        STOP
    };
    struct statQuery {
        QueryType type;
        std::string data;
    };
    std::vector<statQuery> queries_;
    static void trim(std::string_view& sv);
};

} // end namespace stat
    
} // end namespace transport
