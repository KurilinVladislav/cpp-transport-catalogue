#include "request_handler.h"

namespace transport {

RequestHandler::RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer) {
}

std::optional<BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
    auto result = db_.GetBusInfo(bus_name);
    if (result.name == "") {
        return std::nullopt;
    }
    return result;
}


std::optional<std::unordered_set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    const auto& stop_to_buses = db_.GetStopToBuses();
    if (!db_.FindStop(stop_name)) {
        return std::nullopt;
    } else if (stop_to_buses.count(stop_name) == 0) {
        return std::unordered_set<std::string_view>{};
    } else {
        if (stop_to_buses.at(stop_name).size() == 0) {
            return std::unordered_set<std::string_view>{};
        }
        return stop_to_buses.at(stop_name);
    }
}
    
svg::Document RequestHandler::RenderMap() const {

    const auto& buses = db_.GetBuses();
    std::vector<const Bus*> bus_ptrs;
    for(const auto& bus: buses) {
        if (bus.stops.size() != 0) {
            bus_ptrs.push_back(&bus);
        }
    }
    std::sort(bus_ptrs.begin(), bus_ptrs.end(),
             [](const auto& lhs, const auto& rhs){
                 return lhs->name < rhs->name;
             });

    return renderer_.RenderMap(bus_ptrs);
}


} // end namespace transport
