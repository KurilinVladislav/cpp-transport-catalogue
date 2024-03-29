#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "geo.h"
#include "json.h"
#include "request_handler.h"
#include "transport_router.h"

namespace transport {
namespace io {
    
class JsonReader {
public:
    JsonReader(TransportCatalogue& db, const RequestHandler& handler, renderer::MapRenderer& renderer, TransportRouter& router);
    void ReadJsonFromStream(std::istream& input);
    void FillDB();
    void ProcessAndApplyRenderSettings();
    void ProcessAndApplyRouterSettings();
    json::Dict ProcessSerializationSettings() const;
    void ProcessStatRequests(std::ostream& output);
    
private:
    json::Document requests_ = json::Document{nullptr};
    TransportCatalogue& db_;
    const RequestHandler& handler_;
    renderer::MapRenderer& renderer_;
    TransportRouter& router_;
    
    json::Dict ProcessStopInfoRequest(const json::Dict& query) const;
    json::Dict ProcessBusInfoRequest(const json::Dict& query) const;
    json::Dict ProcessMapRequest(const json::Dict& query) const;
    json::Dict ProcessRouteRequest(const json::Dict& query) const;
    svg::Color GetColorFromJsonNode(const json::Node& node) const;
};
    
} // end namespace io
} // end namespace transport
