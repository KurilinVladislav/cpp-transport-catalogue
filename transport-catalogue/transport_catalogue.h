#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"
#include "domain.h"

namespace transport {

class TransportCatalogue {
public:
    TransportCatalogue() {}
    void AddStop(std::string_view name, geo::Coordinates coordinates);
    void AddBus(std::string_view name, std::vector<std::string_view>& stops, bool looped = false);
    void SetDistance(const Stop* from, const Stop* to, int distance);
    int GetDistance(const Stop* from, const Stop* to) const;
    const Stop* FindStop(std::string_view name) const;
    const Bus* FindBus(std::string_view name) const;
    const BusInfo GetBusInfo(std::string_view name) const;
    const std::unordered_map<std::string_view, std::unordered_set<std::string_view>>& GetStopToBuses() const;
    const std::deque<Bus>& GetBuses() const;

private:

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopHasher> distances_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopHasher> geo_distances_;
    std::unordered_map<std::string_view, std::pair<int, double>> busname_to_total_distances_;

};

} // end namespace transport
