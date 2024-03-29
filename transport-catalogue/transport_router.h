#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "router.h"

#include <string_view>
#include <optional>
#include <memory>

namespace transport {
    
struct RouterSettings {
    size_t bus_wait_time;
    double bus_velocity;
};
    
class TransportRouter {
public:
    TransportRouter(const TransportCatalogue& db);
    void SetPointers(std::unique_ptr<graph::DirectedWeightedGraph<double>> g_ptr, std::unique_ptr<graph::Router<double>> r_ptr);
    void ApplySettings(const RouterSettings& settings);
    int GetBusWaitTime() const;
    double GetBusVelocity() const;
    const graph::Edge<double>& GetEdge(size_t id) const;
    void Init();
    std::optional<graph::Router<double>::RouteInfo> BuildRoute(std::string_view from, std::string_view to);
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    const graph::Router<double>& GetRouter() const;

    
private:
    void BuildGraph();

    size_t bus_wait_time_ = 1;
    double bus_velocity_ = 1.0;
    const TransportCatalogue& db_;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;
};
    
} // end namespace transport
