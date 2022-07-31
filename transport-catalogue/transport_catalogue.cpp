#include "transport_catalogue.h"

#include <unordered_set>

#include <iostream>

namespace transport {

void TransportCatalogue::AddStop(std::string_view stopname, geo::Coordinates coordinates) {
    size_t id = stops_.size();
    stops_.push_back({id, static_cast<std::string>(stopname), coordinates});
    std::string_view name = stops_.back().name;
    stopname_to_stop_[name] = &stops_.back();
    if (stop_to_buses_.count(name) == 0) {
        stop_to_buses_[name] = {};
    }
    stop_id_to_stop_[id] = &stops_.back();
}

void TransportCatalogue::AddBus(std::string_view busname, std::vector<std::string_view>& stops, bool looped) {
    size_t id = buses_.size();
    buses_.push_back({id, static_cast<std::string>(busname), {}, looped});
    std::string_view name = buses_.back().name;
    busname_to_bus_[name] = &buses_.back();
    std::vector<const Stop*> stop_ptrs;
    for(const auto& sv: stops) {
        stop_ptrs.push_back(FindStop(sv));
        stop_to_buses_[sv].insert(name);
    }
    buses_.back().stops = stop_ptrs;
    bus_id_to_bus_[id] = &buses_.back();
    
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

double TransportCatalogue::GetGeoDistance(const Stop* from, const Stop* to) const {
    if (geo_distances_.count({from, to}) != 0) {
        return geo_distances_.at({from, to});
    } else {
        return geo_distances_.at({to, from}); // exception may be thrown
    }
}

const Stop* TransportCatalogue::FindStop(std::string_view name) const {
    if (stopname_to_stop_.count(name) != 0) {
        return stopname_to_stop_.at(name);
    } else {
        return nullptr;
    }
}
    
const Stop* TransportCatalogue::GetStopById(size_t id) const {
    if (stop_id_to_stop_.count(id) != 0) {
        return stop_id_to_stop_.at(id);
    } else {
        return nullptr;
    }
}

const Bus* TransportCatalogue::FindBus(std::string_view name) const {
    if (busname_to_bus_.count(name) != 0) {
        return busname_to_bus_.at(name);
    } else {
        return nullptr;
    }
}

const Bus* TransportCatalogue::GetBusById(size_t id) const {
    if (bus_id_to_bus_.count(id) != 0) {
        return bus_id_to_bus_.at(id);
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
    
const std::deque<Stop>& TransportCatalogue::GetStops() const {
    return stops_;
}
    
const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}

void TransportCatalogue::LoadData(const CatalogueSaveData& data) {
    // load data from struct
    stops_ = std::move(data.stops);
    for (const Stop& s : stops_) {
        stopname_to_stop_[s.name] = &s;
        stop_id_to_stop_[s.id] = &s;
    }
    for (const CatalogueSaveData::Bus& b : data.buses) {
        buses_.push_back({b.id, b.name, {}, b.is_roundtrip});
        for(size_t id: b.stop_ids) {
            buses_.back().stops.push_back(GetStopById(id));
        }
    }
    for (const Bus& b : buses_) {
        busname_to_bus_[b.name] = &b;
        bus_id_to_bus_[b.id] = &b;
    }
    for (const CatalogueSaveData::StopToBuses& s : data.stop_to_buses) {
        for (size_t bus_id : s.bus_ids) {
            stop_to_buses_[GetStopById(s.id)->name].insert(GetBusById(bus_id)->name);
        }
    }
    for (const CatalogueSaveData::Distance& d : data.distances) {
        std::pair<const Stop*, const Stop*> p{GetStopById(d.from), GetStopById(d.to)};
        distances_[p] = d.distance;
    }
    for (const CatalogueSaveData::GeoDistance& d : data.geo_distances) {
        std::pair<const Stop*, const Stop*> p{GetStopById(d.from), GetStopById(d.to)};
        geo_distances_[p] = d.distance;
    }
    for (const CatalogueSaveData::BusToTotal& d : data.bus_id_to_total_distances) {
        busname_to_total_distances_[GetBusById(d.id)->name] = {d.distance, d.geo_distance};
    }
}

CatalogueSaveData TransportCatalogue::SaveData() const {
    CatalogueSaveData r;
    r.stops = {}; // serializer gets this straight from db_

    for (const Bus& bus: buses_) {
        std::vector<size_t> ids;
        for (const Stop* s: bus.stops) {
            ids.push_back(s->id);
        }
        CatalogueSaveData::Bus b{bus.id, bus.name, ids, bus.is_roundtrip};
        r.buses.push_back(std::move(b));
    }
    for (const auto& [name, bus_names]: stop_to_buses_) {
        size_t id = FindStop(name)->id;
        std::vector<size_t> bus_ids;
        for (const auto& n : bus_names) {
            bus_ids.push_back(FindBus(n)->id);
        }
        CatalogueSaveData::StopToBuses s{id, bus_ids};
        r.stop_to_buses.push_back(std::move(s));
    }
    for (const auto& [stop_pair, dist]: distances_) {
        size_t from = stop_pair.first->id;
        size_t to = stop_pair.second->id;
        CatalogueSaveData::Distance d{from, to, dist};
        r.distances.push_back(std::move(d));
    }
    for (const auto& [stop_pair, dist]: geo_distances_) {
        size_t from = stop_pair.first->id;
        size_t to = stop_pair.second->id;
        CatalogueSaveData::GeoDistance d{from, to, dist};
        r.geo_distances.push_back(std::move(d));
    }
    for (const auto& [name, pair_dist]: busname_to_total_distances_) {
        size_t id = FindBus(name)->id;
        CatalogueSaveData::BusToTotal d{id, pair_dist.first, pair_dist.second};
        r.bus_id_to_total_distances.push_back(std::move(d));
    }

    return r;
}

} // end namespace transport
