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
    router_serialize::Graph SerGraph();
    router_serialize::RouterSettings SerRouter();
    void DserGraph(const router_serialize::Graph& graph);
    void DserRouter(const router_serialize::RoutesInternalData& data);

    transport_serialize::TransportCatalogue SerCatalogue();
    renderer_serialize::RenderSettings SerRenderer();
    void DserCatalogue(const transport_serialize::TransportCatalogue& db);
    void DserRenderer(const renderer_serialize::RenderSettings& settings);

    renderer_serialize::Color SerColor(svg::Color color);
    svg::Color DserColor(renderer_serialize::Color color);

    transport_serialize::Stop SerStop(const Stop& stop);
    transport_serialize::Bus SerBus(const CatalogueSaveData::Bus& bus);
    transport_serialize::StopToBuses SerStopToBuses(const CatalogueSaveData::StopToBuses& stb);
    transport_serialize::Distance SerDistance(const CatalogueSaveData::Distance& dist);
    transport_serialize::GeoDistance SerGeoDistance(const CatalogueSaveData::GeoDistance& dist);
    transport_serialize::BusToTotal SerBusToTotal(const CatalogueSaveData::BusToTotal& dist);

    Stop DserStop(const transport_serialize::Stop& stop);
    CatalogueSaveData::Bus DserBus(const transport_serialize::Bus& bus);
    CatalogueSaveData::StopToBuses DserStopToBuses(const transport_serialize::StopToBuses& stb);
    CatalogueSaveData::Distance DserDistance(const transport_serialize::Distance& dist);
    CatalogueSaveData::GeoDistance DserGeoDistance(const transport_serialize::GeoDistance& dist);
    CatalogueSaveData::BusToTotal DserBusToTotal(const transport_serialize::BusToTotal& dist);

    TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
    TransportRouter& router_;
    const io::JsonReader& reader_;
    const std::string file_;
};



} // end namespace transport
