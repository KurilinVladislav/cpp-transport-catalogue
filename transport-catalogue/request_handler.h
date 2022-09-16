#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <optional>

namespace transport {
    
class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusInfo> GetBusStat(const std::string_view& bus_name) const;

    std::optional<std::unordered_set<std::string_view>> GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

private:
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};

} // end namespace transport
