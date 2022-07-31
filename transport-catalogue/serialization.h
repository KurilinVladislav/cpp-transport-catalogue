#pragma once

#include "json_reader.h"

#include <transport_catalogue.pb.h>
#include <svg.pb.h>
#include <map_renderer.pb.h>
#include <graph.pb.h>
#include <transport_router.pb.h>

#include <fstream>

namespace transport {

class Serializer {
public:
    Serializer(TransportCatalogue& db, renderer::MapRenderer& renderer, TransportRouter& router, const io::JsonReader& reader);
    void SaveData();
    void LoadData();

private:
    router_serialize::Graph SerializeGraph();
    router_serialize::RouterSettings SerializeRouter();
    void DeserializeGraph(const router_serialize::Graph& graph);
    void DeserializeRouter(const router_serialize::RoutesInternalData& data);

    transport_serialize::TransportCatalogue SerializeCatalogue();
    renderer_serialize::RenderSettings SerializeRenderer();
    void DeserializeCatalogue(const transport_serialize::TransportCatalogue& db);
    void DeserializeRenderer(const renderer_serialize::RenderSettings& settings);

    renderer_serialize::Color SerializeColor(svg::Color color);
    svg::Color DeserializeColor(renderer_serialize::Color color);

    transport_serialize::Stop SerializeStop(const Stop& stop);
    transport_serialize::Bus SerializeBus(const CatalogueSaveData::Bus& bus);
    transport_serialize::StopToBuses SerializeStopToBuses(const CatalogueSaveData::StopToBuses& stb);
    transport_serialize::Distance SerializeDistance(const CatalogueSaveData::Distance& dist);
    transport_serialize::GeoDistance SerializeGeoDistance(const CatalogueSaveData::GeoDistance& dist);
    transport_serialize::BusToTotal SerializeBusToTotal(const CatalogueSaveData::BusToTotal& dist);

    Stop DeserializeStop(const transport_serialize::Stop& stop);
    CatalogueSaveData::Bus DeserializeBus(const transport_serialize::Bus& bus);
    CatalogueSaveData::StopToBuses DeserializeStopToBuses(const transport_serialize::StopToBuses& stb);
    CatalogueSaveData::Distance DeserializeDistance(const transport_serialize::Distance& dist);
    CatalogueSaveData::GeoDistance DeserializeGeoDistance(const transport_serialize::GeoDistance& dist);
    CatalogueSaveData::BusToTotal DeserializeBusToTotal(const transport_serialize::BusToTotal& dist);

    TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
    TransportRouter& router_;
    const io::JsonReader& reader_;
    const std::string file_;
};



} // end namespace transport
