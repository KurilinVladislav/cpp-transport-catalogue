#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "geo.h"

namespace transport {
    
namespace input {

struct StopQuery {
    std::string_view name;
    Coordinates coordinates;
    std::unordered_map<std::string_view, int> distances;
};

struct BusQuery {
    std::string_view name;
    std::vector<std::string_view> stops;
};

class InputReader {
public:
    InputReader() {}

    void Read(std::istream& input);

    std::vector<StopQuery> ProcessStopQueries() const;
    std::vector<BusQuery> ProcessBusQueries() const;

private:
    std::vector<std::string> stop_queries_;
    std::vector<std::string> bus_queries_;
    
    static void trim(std::string_view& sv);

};

} // end namespace input

} // end namespace transport
