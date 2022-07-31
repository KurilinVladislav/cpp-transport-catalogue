#pragma once

#include "ranges.h"

#include <cstdlib>
#include <vector>

#include <graph.pb.h>


namespace graph {

using VertexId = size_t;
using EdgeId = size_t;

template <typename Weight>
struct Edge {
    VertexId from;
    VertexId to;
    Weight weight;
    size_t bus_id;
    size_t stop_count;
};

template <typename Weight>
class DirectedWeightedGraph {
private:
    using IncidenceList = std::vector<EdgeId>;
    using IncidentEdgesRange = ranges::Range<typename IncidenceList::const_iterator>;

public:
    DirectedWeightedGraph() = default;
    explicit DirectedWeightedGraph(size_t vertex_count);
    DirectedWeightedGraph(const router_serialize::Graph& graph);
    EdgeId AddEdge(const Edge<Weight>& edge);

    size_t GetVertexCount() const;
    size_t GetEdgeCount() const;
    const Edge<Weight>& GetEdge(EdgeId edge_id) const;
    IncidentEdgesRange GetIncidentEdges(VertexId vertex) const;

    router_serialize::Graph SerGraph() const;

private:
    std::vector<Edge<Weight>> edges_;
    std::vector<IncidenceList> incidence_lists_;
};

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count)
    : incidence_lists_(vertex_count) {
}

template <typename Weight>
DirectedWeightedGraph<Weight>::DirectedWeightedGraph(const router_serialize::Graph& graph) {
    for (int i = 0; i < graph.edges_size(); ++i) {
        Edge<double> e;
        e.bus_id = graph.edges(i).bus_id();
        e.from = graph.edges(i).from();
        e.to = graph.edges(i).to();
        e.weight = graph.edges(i).weight();
        e.stop_count = graph.edges(i).stop_count();
        edges_.push_back(std::move(e));
    }
    for (int i = 0; i < graph.incidence_lists_size(); ++i) {
        IncidenceList list;
        for (int j = 0; j < graph.incidence_lists(i).edge_ids_size(); ++j) {
            list.push_back(graph.incidence_lists(i).edge_ids(j));
        }
        incidence_lists_.push_back(std::move(list));
    }
}

template <typename Weight>
EdgeId DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) {
    edges_.push_back(edge);
    const EdgeId id = edges_.size() - 1;
    incidence_lists_.at(edge.from).push_back(id);
    return id;
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetVertexCount() const {
    return incidence_lists_.size();
}

template <typename Weight>
size_t DirectedWeightedGraph<Weight>::GetEdgeCount() const {
    return edges_.size();
}

template <typename Weight>
const Edge<Weight>& DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const {
    return edges_.at(edge_id);
}

template <typename Weight>
typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const {
    return ranges::AsRange(incidence_lists_.at(vertex));
}

template<typename Weight>
router_serialize::Graph DirectedWeightedGraph<Weight>::SerGraph() const {
    router_serialize::Graph g;

    for(size_t i = 0; i < incidence_lists_.size(); ++i) {
        router_serialize::IncidenceList list;
        for(size_t j = 0; j < incidence_lists_.at(i).size(); ++j) {
            list.add_edge_ids(incidence_lists_.at(i).at(j));
        }
        g.add_incidence_lists();
        *g.mutable_incidence_lists((int)i) = std::move(list);
    }
    for(size_t i = 0; i < edges_.size(); ++i) {
        router_serialize::Edge e;
        e.set_from(edges_.at(i).from);
        e.set_to(edges_.at(i).to);
        e.set_bus_id(edges_.at(i).bus_id);
        e.set_weight(edges_.at(i).weight);
        e.set_stop_count(edges_.at(i).stop_count);
        g.add_edges();
        *g.mutable_edges(i) = std::move(e);
    }
    return g;
}

}  // namespace graph
