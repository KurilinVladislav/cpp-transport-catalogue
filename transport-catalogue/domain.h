#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "geo.h"

namespace transport {

struct Stop {
    size_t id;
    std::string name;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops; // all stops, including way back
    bool is_roundtrip;
};

struct BusInfo {
    std::string_view name;
    int uniqueStopsCount;
    int totalStopsCount;
    int routeLength;
    double curvature;
};

class StopHasher {
    public:
    size_t operator()(std::pair<const Stop*, const Stop*> stops) const;
    size_t operator()(const Stop* stop) const;
};
    
} // end namespace transport
