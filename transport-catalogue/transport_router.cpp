#include "transport_router.h"

#include <iostream>

namespace transport {
    
TransportRouter::TransportRouter(const TransportCatalogue& db)
    : db_(db) {
}

void TransportRouter::SetPointers(std::unique_ptr<graph::DirectedWeightedGraph<double>> g_ptr, std::unique_ptr<graph::Router<double>> r_ptr) {
    graph_ = std::move(g_ptr);
    router_ = std::move(r_ptr);
}
    
void TransportRouter::ApplySettings(const RouterSettings& s) {
    bus_wait_time_ = s.bus_wait_time;
    bus_velocity_ = s.bus_velocity;
}
    
int TransportRouter::GetBusWaitTime() const {
    return static_cast<int>(bus_wait_time_);
}
    
double TransportRouter::GetBusVelocity() const {
    return bus_velocity_;
}
    
const graph::Edge<double>& TransportRouter::GetEdge(size_t id) const {
    return graph_->GetEdge(id);
}

void TransportRouter::Init() {
    if (router_ == nullptr) { // if called for the first time, create graph and router
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(db_.GetStops().size());
        BuildGraph();
        router_ = std::make_unique<graph::Router<double>>(*graph_);
    }
}

void TransportRouter::BuildGraph() {
    const auto& buses = db_.GetBuses();
    for (const auto& bus: buses) {
        size_t size = bus.stops.size();
        if (size == 0) {
            continue;
        }

        auto build_part = [&](size_t start, size_t finish) {
            for(size_t i = start; i < finish; ++i) {
                double dist = 0;
                for(size_t j = i + 1; j < finish; ++j) {
                    double delta = 0;
                    try {
                        delta = db_.GetDistance(bus.stops[j-1], bus.stops[j]);
                    } catch (...) {
                        try {
                            delta = db_.GetGeoDistance(bus.stops[j-1], bus.stops[j]);
                        } catch (...) {
                            std::cerr << "Distance from " << bus.stops[j-1]->name
                                << " to " << bus.stops[j]->name << " not found" << std::endl;
                            throw;
                        }
                    }
                    dist += delta;
                    double weight = bus_wait_time_ + dist * 0.06 / bus_velocity_;
                    graph_->AddEdge({bus.stops[i]->id, bus.stops[j]->id, weight, bus.id, j-i});
                }
            }
        };
        if (bus.is_roundtrip == false) {
            build_part(0, (size + 1) / 2); // last is not included so +1
            build_part((size - 1) / 2, size); // start from middle
        } else {
            build_part(0, size);
        }
    }
}
    
std::optional<graph::Router<double>::RouteInfo> TransportRouter::BuildRoute(std::string_view from, std::string_view to) {
    /*if (router_ == nullptr) { // if called for the first time, create graph and router
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(db_.GetStops().size());
        BuildGraph();
        router_ = std::make_unique<graph::Router<double>>(*graph_);
    }*/
    Init();
    const Stop* from_ptr = db_.FindStop(from);
    const Stop* to_ptr = db_.FindStop(to);
    if (from_ptr == nullptr || to_ptr == nullptr) {
        return std::nullopt;
    }
    return router_->BuildRoute(from_ptr->id, to_ptr->id);
}

const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
    return *graph_;
}

const graph::Router<double>& TransportRouter::GetRouter() const {
    return *router_;
}
    
} // end namespace transport
