#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

#include "geo.h"
#include "input_reader.h"

namespace transport {

class TransportCatalogue {
    struct Stop {
        std::string name;
        Coordinates coordinates;
    };
    struct Bus {
        std::string name;
        std::vector<const Stop*> stops;
    };
    struct BusInfo {
        std::string_view name;
        int uniqueStopsCount;
        int totalStopsCount;
        int routeLength;
        double curvature;
    };

public:
    TransportCatalogue() {}
    void AddStop(std::string_view name, Coordinates coordinates);
    void AddBus(std::string_view name, std::vector<std::string_view>& stops);
    void SetDistance(const Stop* from, const Stop* to, int distance);
    int GetDistance(const Stop* from, const Stop* to) const;
    const Stop* FindStop(std::string_view name) const;
    const Bus* FindBus(std::string_view name) const;
    const BusInfo GetBusInfo(std::string_view name) const;
    const std::unordered_map<std::string_view, std::unordered_set<std::string_view>>& GetStopToBuses() const;

    void FillInfo(const input::InputReader& reader);

private:
    class StopHasher {
    public:
        size_t operator()(std::pair<const Stop*, const Stop*> stops) const {
            return std::hash<const void*>{}(stops.first) + 37 * std::hash<const void*>{}(stops.second);
        }
        size_t operator()(const Stop* stop) const {
            return std::hash<const void*>{}(stop);
        }
    };

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopHasher> distances_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopHasher> geo_distances_;

};

} // end namespace transport
