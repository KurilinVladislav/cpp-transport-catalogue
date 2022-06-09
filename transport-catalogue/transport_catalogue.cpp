#include "transport_catalogue.h"

#include <unordered_set>

namespace transport {

void TransportCatalogue::AddStop(std::string_view stopname, geo::Coordinates coordinates) {
    stops_.push_back({static_cast<std::string>(stopname), coordinates});
    std::string_view name = stops_.back().name;
    stopname_to_stop_[name] = &stops_.back();
    if (stop_to_buses_.count(name) == 0) {
        stop_to_buses_[name] = {};
    }
}

void TransportCatalogue::AddBus(std::string_view busname, std::vector<std::string_view>& stops, bool looped) {
    buses_.push_back({static_cast<std::string>(busname), {}, looped});
    std::string_view name = buses_.back().name;
    busname_to_bus_[name] = &buses_.back();
    std::vector<const Stop*> stop_ptrs;
    for(const auto& sv: stops) {
        stop_ptrs.push_back(FindStop(sv));
        stop_to_buses_[sv].insert(name);
    }
    buses_.back().stops = stop_ptrs;
    
    int total_distance = 0;
    double total_geo_distance = 0.0;
    for(size_t i = 1; i < stop_ptrs.size(); ++i) {
        double distance = ComputeDistance(stop_ptrs[i-1]->coordinates, stop_ptrs[i]->coordinates);
        std::pair<const Stop*, const Stop*> pair;
        pair.first = stop_ptrs[i-1];
        pair.second = stop_ptrs[i];
        geo_distances_[pair] = distance;
        total_distance += GetDistance(stop_ptrs[i-1], stop_ptrs[i]);
        total_geo_distance += distance;
    }
    busname_to_total_distances_[name] = {total_distance, total_geo_distance};
}
    
void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, int distance) {
    if (distances_.count({from, to}) == 0) {
        distances_[{from, to}] = distance;
        distances_[{to, from}] = distance;
    } else {
        distances_.at({from, to}) = distance;
    }
    
}

int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    if (distances_.count({from, to}) != 0) {
        return distances_.at({from, to});
    } else {
        return distances_.at({to, from}); // exception may be thrown
    }
}


const Stop* TransportCatalogue::FindStop(std::string_view name) const {
    return stopname_to_stop_.at(name);
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    if (busname_to_bus_.count(name) != 0) {
        return busname_to_bus_.at(name);
    } else {
        return nullptr;
    }
}

const BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
    BusInfo info{};
    const Bus* bus_ptr = FindBus(name);
    if (bus_ptr != nullptr) {
        info.name = name;
        std::unordered_set<const Stop*> unique_stops(bus_ptr->stops.begin(), bus_ptr->stops.end());
        info.uniqueStopsCount = unique_stops.size();
        info.totalStopsCount = bus_ptr->stops.size();
        int total_distance = busname_to_total_distances_.at(name).first;
        double total_geo_distance = busname_to_total_distances_.at(name).second;
        info.routeLength = total_distance;
        info.curvature = (1.0f * total_distance) / total_geo_distance;
    }
    return info;
}
    
const std::unordered_map<std::string_view, std::unordered_set<std::string_view>>& TransportCatalogue::GetStopToBuses() const {
    return stop_to_buses_;
}
    
const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}


} // end namespace transport
