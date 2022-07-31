#pragma once

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

#include <iostream> // for testing Print()

#include "geo.h"
#include "domain.h"

namespace transport {

struct CatalogueSaveData {
    struct Bus {
        size_t id;
        std::string name;
        std::vector<size_t> stop_ids;
        bool is_roundtrip;
    };
    struct StopToBuses {
        size_t id;
        std::vector<size_t> bus_ids;
    };
    struct Distance {
        size_t from;
        size_t to;
        int distance;
    };
    struct GeoDistance {
        size_t from;
        size_t to;
        double distance;
    };
    struct BusToTotal {
        size_t id;
        int distance;
        double geo_distance;
    };
    std::deque<Stop> stops;
    std::deque<Bus> buses;
    std::vector<StopToBuses> stop_to_buses;
    std::vector<Distance> distances;
    std::vector<GeoDistance> geo_distances;
    std::vector<BusToTotal> bus_id_to_total_distances;
};

class TransportCatalogue {
public:

    TransportCatalogue() {}
    void AddStop(std::string_view name, geo::Coordinates coordinates);
    void AddBus(std::string_view name, std::vector<std::string_view>& stops, bool looped = false);
    void SetDistance(const Stop* from, const Stop* to, int distance);
    int GetDistance(const Stop* from, const Stop* to) const;
    double GetGeoDistance(const Stop* from, const Stop* to) const;
    const Stop* FindStop(std::string_view name) const;
    const Stop* GetStopById(size_t id) const;
    const Bus* FindBus(std::string_view name) const;
    const Bus* GetBusById(size_t id) const;
    const BusInfo GetBusInfo(std::string_view name) const;
    const std::unordered_map<std::string_view, std::unordered_set<std::string_view>>& GetStopToBuses() const;
    const std::deque<Stop>& GetStops() const;
    const std::deque<Bus>& GetBuses() const;
    void LoadData(const CatalogueSaveData& data);
    CatalogueSaveData SaveData() const;
    
    void Print() const {
        std::cout << "Stops:" << std::endl;
        for (const auto& stop: stops_) {
            std::cout << "   id " << stop.id << ", name = " << stop.name << std::endl;
        }
    }

private:

    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<size_t, const Stop*> stop_id_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<size_t, const Bus*> bus_id_to_bus_;
    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stop_to_buses_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopHasher> distances_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, StopHasher> geo_distances_;
    std::unordered_map<std::string_view, std::pair<int, double>> busname_to_total_distances_;

};

} // end namespace transport
