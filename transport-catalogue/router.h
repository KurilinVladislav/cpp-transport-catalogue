#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include <transport_router.pb.h>

namespace graph {

template <typename Weight>
class Router {
private:
    using Graph = DirectedWeightedGraph<Weight>;

public:
    explicit Router(const Graph& graph);
    Router(const Graph& graph, const router_serialize::RoutesInternalData& data);

    struct RouteInfo {
        Weight weight;
        std::vector<EdgeId> edges;
    };

    std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;

    router_serialize::RoutesInternalData SerializeRoutesInternalData() const;
    const Graph& GetGraph() const;

private:
    struct RouteInternalData {
        Weight weight;
        std::optional<EdgeId> prev_edge;
    };
    using RoutesInternalData = std::vector<std::vector<std::optional<RouteInternalData>>>;

    void InitializeRoutesInternalData(const Graph& graph) {
        const size_t vertex_count = graph.GetVertexCount();
        for (VertexId vertex = 0; vertex < vertex_count; ++vertex) {
            routes_internal_data_[vertex][vertex] = RouteInternalData{ZERO_WEIGHT, std::nullopt};
            for (const EdgeId edge_id : graph.GetIncidentEdges(vertex)) {
                const auto& edge = graph.GetEdge(edge_id);
                if (edge.weight < ZERO_WEIGHT) {
                    throw std::domain_error("Edges' weights should be non-negative");
                }
                auto& route_internal_data = routes_internal_data_[vertex][edge.to];
                if (!route_internal_data || route_internal_data->weight > edge.weight) {
                    route_internal_data = RouteInternalData{edge.weight, edge_id};
                }
            }
        }
    }

    void RelaxRoute(VertexId vertex_from, VertexId vertex_to, const RouteInternalData& route_from,
                    const RouteInternalData& route_to) {
        auto& route_relaxing = routes_internal_data_[vertex_from][vertex_to];
        const Weight candidate_weight = route_from.weight + route_to.weight;
        if (!route_relaxing || candidate_weight < route_relaxing->weight) {
            route_relaxing = {candidate_weight,
                              route_to.prev_edge ? route_to.prev_edge : route_from.prev_edge};
        }
    }

    void RelaxRoutesInternalDataThroughVertex(size_t vertex_count, VertexId vertex_through) {
        for (VertexId vertex_from = 0; vertex_from < vertex_count; ++vertex_from) {
            if (const auto& route_from = routes_internal_data_[vertex_from][vertex_through]) {
                for (VertexId vertex_to = 0; vertex_to < vertex_count; ++vertex_to) {
                    if (const auto& route_to = routes_internal_data_[vertex_through][vertex_to]) {
                        RelaxRoute(vertex_from, vertex_to, *route_from, *route_to);
                    }
                }
            }
        }
    }

    static constexpr Weight ZERO_WEIGHT{};
    const Graph& graph_;
    RoutesInternalData routes_internal_data_;
};

template <typename Weight>
Router<Weight>::Router(const Graph& graph)
    : graph_(graph)
    , routes_internal_data_(graph.GetVertexCount(),
                            std::vector<std::optional<RouteInternalData>>(graph.GetVertexCount()))
{
    InitializeRoutesInternalData(graph);

    const size_t vertex_count = graph.GetVertexCount();
    for (VertexId vertex_through = 0; vertex_through < vertex_count; ++vertex_through) {
        RelaxRoutesInternalDataThroughVertex(vertex_count, vertex_through);
    }
}

template <typename Weight>
Router<Weight>::Router(const Graph& graph, const router_serialize::RoutesInternalData& data)
    : graph_(graph) {
    // parse here and we're done
    for(int i = 0; i < data.items_size(); ++i) {
        std::vector<std::optional<RouteInternalData>> v;
        for (int j = 0; j < data.items(i).items_size(); ++j) {
            std::optional<RouteInternalData> opt_i_d;
            if(data.items(i).items(j).data_size() != 0) {
                RouteInternalData i_d;
                const auto& v = data.items(i).items(j).data(0);
                i_d.weight = v.weight();
                if (v.prev_edge_size() != 0) {
                    i_d.prev_edge = v.prev_edge(0);
                } else {
                    i_d.prev_edge = std::nullopt;
                }
                opt_i_d = i_d;
            } else {
                opt_i_d = std::nullopt;
            }
            v.push_back(std::move(opt_i_d));
        }
        routes_internal_data_.push_back(std::move(v));
    }
}

template <typename Weight>
std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from,
                                                                             VertexId to) const {
    const auto& route_internal_data = routes_internal_data_.at(from).at(to);
    if (!route_internal_data) {
        return std::nullopt;
    }
    const Weight weight = route_internal_data->weight;
    std::vector<EdgeId> edges;
    for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
         edge_id;
         edge_id = routes_internal_data_[from][graph_.GetEdge(*edge_id).from]->prev_edge)
    {
        edges.push_back(*edge_id);
    }
    std::reverse(edges.begin(), edges.end());

    return RouteInfo{weight, std::move(edges)};
}

template<typename Weight>
router_serialize::RoutesInternalData Router<Weight>::SerializeRoutesInternalData() const {
    router_serialize::RoutesInternalData d;
    for (size_t i = 0; i < routes_internal_data_.size(); ++i) {
        router_serialize::VectorOpt vector_opt;
        for (size_t j = 0; j < routes_internal_data_.at(i).size(); ++j) {
            router_serialize::OptInternalData opt_i_d;
            if (routes_internal_data_.at(i).at(j).has_value()) {
                const auto& v = *routes_internal_data_.at(i).at(j);
                router_serialize::InternalData i_d;
                i_d.set_weight(v.weight);
                if (v.prev_edge.has_value()) {
                    i_d.add_prev_edge(*v.prev_edge);
                }
                opt_i_d.add_data();
                *opt_i_d.mutable_data(0) = std::move(i_d);
            }
            vector_opt.add_items();
            *vector_opt.mutable_items((int)j) = std::move(opt_i_d);
        }
        d.add_items();
        *d.mutable_items((int)i) = std::move(vector_opt);
    }
    return d;
}

template<typename Weight>
const DirectedWeightedGraph<Weight>& Router<Weight>::GetGraph() const {
    return graph_;
}

}  // namespace graph
