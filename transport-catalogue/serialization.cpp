#include "serialization.h"

namespace transport {

Serializer::Serializer(TransportCatalogue& db, renderer::MapRenderer& renderer, TransportRouter& router, const io::JsonReader& reader)
    : db_(db)
    , renderer_(renderer)
    , router_(router)
    , reader_(reader)
    , file_(reader_.ProcessSerializationSettings().at("file").AsString()) {
}

void Serializer::SaveData() {
    std::ofstream out(file_.c_str(), std::ios::binary);
    if (!out) {
        std::cerr << "Couldn't open output file " << file_ << ", not saving." << std::endl;
        return;
    }
    transport_serialize::SaveData savedata;
    *savedata.mutable_transport_catalogue() = std::move(SerCatalogue());
    *savedata.mutable_render_settings() = std::move(SerRenderer());
    *savedata.mutable_graph() = std::move(SerGraph());
    *savedata.mutable_router_data() = std::move(SerRouter());

    savedata.SerializeToOstream(&out);
}

void Serializer::LoadData() {
    std::ifstream in(file_.c_str(), std::ios::binary);
    if (!in) {
        std::cerr << "Couldn't load input file " << file_ << ", no loading will be done." << std::endl;
        return;
    }
    transport_serialize::SaveData savedata;
    if (!savedata.ParseFromIstream(&in)) {
        std::cerr << "Couldn't parse file." << std::endl;
        return;
    }

    transport_serialize::TransportCatalogue catalogue = std::move(savedata.transport_catalogue());
    renderer_serialize::RenderSettings render_settings = std::move(savedata.render_settings());
    router_serialize::Graph graph = std::move(savedata.graph());
    router_serialize::RouterSettings router_settings = std::move(savedata.router_data());

    DserCatalogue(catalogue);
    DserRenderer(render_settings);
    router_.ApplySettings({router_settings.bus_wait_time(), router_settings.bus_velocity()});
    auto graph_ptr = std::make_unique<graph::DirectedWeightedGraph<double>>(graph);
    auto router_ptr = std::make_unique<graph::Router<double>>(*graph_ptr, router_settings.data());
    router_.SetPointers(std::move(graph_ptr), std::move(router_ptr));
}

transport_serialize::TransportCatalogue Serializer::SerCatalogue() {
    transport_serialize::TransportCatalogue c;
    CatalogueSaveData savedata = std::move(db_.SaveData());

    int i = 0;
    for (const Stop& s : db_.GetStops()) {
        c.add_stops();
        *c.mutable_stops(i) = std::move(SerStop(s));
        ++i;
    }
    i = 0;
    for (const CatalogueSaveData::Bus& b : savedata.buses) {
        c.add_buses();
        *c.mutable_buses(i) = std::move(SerBus(b));
        ++i;
    }
    i = 0;
    for (const CatalogueSaveData::StopToBuses& s : savedata.stop_to_buses) {
        c.add_stop_to_buses();
        *c.mutable_stop_to_buses(i) = std::move(SerStopToBuses(s));
        ++i;
    }
    i = 0;
    for (const CatalogueSaveData::Distance& d: savedata.distances) {
        c.add_distances();
        *c.mutable_distances(i) = std::move(SerDistance(d));
        ++i;
    }
    i = 0;
    for (const CatalogueSaveData::GeoDistance& d: savedata.geo_distances) {
        c.add_geo_distances();
        *c.mutable_geo_distances(i) = std::move(SerGeoDistance(d));
        ++i;
    }
    i = 0;
    for (const CatalogueSaveData::BusToTotal& d: savedata.bus_id_to_total_distances) {
        c.add_bus_id_to_total_distances();
        *c.mutable_bus_id_to_total_distances(i) = std::move(SerBusToTotal(d));
        ++i;
    }

    return c;
}

renderer_serialize::RenderSettings Serializer::SerRenderer() {
    renderer_serialize::RenderSettings r;
    const auto& s = renderer_.GetSettings();

    r.set_width(s.width);
    r.set_height(s.height);
    r.set_padding(s.padding);
    r.set_line_width(s.line_width);
    r.set_stop_radius(s.stop_radius);
    r.set_bus_label_font_size(s.bus_label_font_size);
    renderer_serialize::Point p;
    p.set_x(s.bus_label_offset.x);
    p.set_y(s.bus_label_offset.y);
    *r.mutable_bus_label_offset() = std::move(p);
    r.set_stop_label_font_size(s.stop_label_font_size);
    p.set_x(s.stop_label_offset.x);
    p.set_y(s.stop_label_offset.y);
    *r.mutable_stop_label_offset() = std::move(p);
    *r.mutable_underlayer_color() = std::move(SerColor(s.underlayer_color));
    r.set_underlayer_width(s.underlayer_width);
    int i = 0;
    for(const svg::Color& col: s.color_palette) {
        r.add_color_palette();
        *r.mutable_color_palette(i) = std::move(SerColor(col));
        ++i;
    }

    return r;
}

router_serialize::Graph Serializer::SerGraph() {
    return router_.GetGraph().SerGraph();
}

router_serialize::RouterSettings Serializer::SerRouter() {
    router_serialize::RouterSettings s;
    *s.mutable_data() = std::move(router_.GetRouter().SerRoutesInternalData());
    s.set_bus_wait_time(router_.GetBusWaitTime());
    s.set_bus_velocity(router_.GetBusVelocity());
    return s;
}

void Serializer::DserCatalogue(const transport_serialize::TransportCatalogue& c) {
    CatalogueSaveData s;
    for (int i = 0; i < c.stops_size(); ++i) {
        s.stops.push_back(std::move(DserStop(c.stops(i))));
    }
    for (int i = 0; i < c.buses_size(); ++i) {
        s.buses.push_back(std::move(DserBus(c.buses(i))));
    }
    for (int i = 0; i < c.stop_to_buses_size(); ++i) {
        s.stop_to_buses.push_back(std::move(DserStopToBuses(c.stop_to_buses(i))));
    }
    for (int i = 0; i < c.distances_size(); ++i) {
        s.distances.push_back(std::move(DserDistance(c.distances(i))));
    }
    for (int i = 0; i < c.geo_distances_size(); ++i) {
        s.geo_distances.push_back(std::move(DserGeoDistance(c.geo_distances(i))));
    }
    for (int i = 0; i < c.bus_id_to_total_distances_size(); ++i) {
        s.bus_id_to_total_distances.push_back(std::move(DserBusToTotal(c.bus_id_to_total_distances(i))));
    }

    db_.LoadData(s);
}

void Serializer::DserRenderer(const renderer_serialize::RenderSettings& c) {
    renderer::RenderSettings s;

    s.width = c.width();
    s.height = c.height();
    s.padding = c.padding();
    s.line_width = c.line_width();
    s.stop_radius = c.stop_radius();
    s.bus_label_font_size = c.bus_label_font_size();
    s.bus_label_offset = std::move(svg::Point{c.bus_label_offset().x(), c.bus_label_offset().y()});
    s.stop_label_font_size = c.stop_label_font_size();
    s.stop_label_offset = std::move(svg::Point{c.stop_label_offset().x(), c.stop_label_offset().y()});
    s.underlayer_color = std::move(DserColor(c.underlayer_color()));
    s.underlayer_width = c.underlayer_width();
    for (int i = 0; i < c.color_palette_size(); ++i) {
        s.color_palette.push_back(std::move(DserColor(c.color_palette(i))));
    }

    renderer_.ApplySettings(s);

}

void Serializer::DserGraph(const router_serialize::Graph& graph) {

}
void Serializer::DserRouter(const router_serialize::RoutesInternalData& data) {

}

////////////////////////////////   Serialization / Deserialization of smaller parts  //////////////////////////

renderer_serialize::Color Serializer::SerColor(svg::Color color) {
    renderer_serialize::Color c;
    c.set_is_rgb(false);
    c.set_is_rgba(false);
    c.set_is_string(false);
    c.set_is_none(false);
    if(std::holds_alternative<svg::Rgb>(color)) {
        c.set_r(std::get<svg::Rgb>(color).red);
        c.set_g(std::get<svg::Rgb>(color).green);
        c.set_b(std::get<svg::Rgb>(color).blue);
        c.set_is_rgb(true);
    } else if(std::holds_alternative<svg::Rgba>(color)) {
        c.set_r(std::get<svg::Rgba>(color).red);
        c.set_g(std::get<svg::Rgba>(color).green);
        c.set_b(std::get<svg::Rgba>(color).blue);
        c.set_a(std::get<svg::Rgba>(color).opacity);
        c.set_is_rgba(true);
    } else if(std::holds_alternative<std::string>(color)) {
        *c.mutable_str() = std::move(std::get<std::string>(color));
        c.set_is_string(true);
    } else {
        c.set_is_none(true);
    }
    return c;
}

svg::Color Serializer::DserColor(renderer_serialize::Color color) {
    svg::Color c;
    if(color.is_rgb()) {
        return svg::Color{svg::Rgb{color.r(), color.g(), color.b()}};
    } else if(color.is_rgba()) {
        return svg::Color{svg::Rgba{color.r(), color.g(), color.b(), color.a()}};
    } else if(color.is_string()) {
        return svg::Color{color.str()};
    } else {
        return svg::NoneColor;
    }
}

transport_serialize::Stop Serializer::SerStop(const Stop& stop) {
    transport_serialize::Stop result;
    transport_serialize::Coordinates coords;
    coords.set_lat(stop.coordinates.lat);
    coords.set_lng(stop.coordinates.lng);
    *result.mutable_coordinates() = std::move(coords);
    result.set_name(stop.name);
    result.set_id(stop.id);
    return result;
}

transport_serialize::Bus Serializer::SerBus(const CatalogueSaveData::Bus& bus) {
    transport_serialize::Bus result;
    result.set_id(bus.id);
    result.set_name(bus.name);
    for(size_t id: bus.stop_ids) {
        result.add_stop_ids(id);
    }
    result.set_is_roundtrip(bus.is_roundtrip);
    return result;
}

transport_serialize::StopToBuses Serializer::SerStopToBuses(const CatalogueSaveData::StopToBuses& stb) {
    transport_serialize::StopToBuses r;
    r.set_id(stb.id);
    for(size_t b_id : stb.bus_ids) {
        r.add_bus_ids(b_id);
    }
    return r;
}

transport_serialize::Distance Serializer::SerDistance(const CatalogueSaveData::Distance& dist) {
    transport_serialize::Distance result;
    result.set_from(dist.from);
    result.set_to(dist.to);
    result.set_distance(dist.distance);
    return result;
}

transport_serialize::GeoDistance Serializer::SerGeoDistance(const CatalogueSaveData::GeoDistance& dist) {
    transport_serialize::GeoDistance r;
    r.set_from(dist.from);
    r.set_to(dist.to);
    r.set_distance(dist.distance);
    return r;
}

transport_serialize::BusToTotal Serializer::SerBusToTotal(const CatalogueSaveData::BusToTotal& dist) {
    transport_serialize::BusToTotal r;
    r.set_id(dist.id);
    r.set_distance(dist.distance);
    r.set_geo_distance(dist.geo_distance);
    return r;
}

Stop Serializer::DserStop(const transport_serialize::Stop& stop) {
    Stop r;
    r.id = stop.id();
    r.name = stop.name();
    r.coordinates = {stop.coordinates().lat(), stop.coordinates().lng()};
    return r;
}

CatalogueSaveData::Bus Serializer::DserBus(const transport_serialize::Bus& bus) {
    CatalogueSaveData::Bus r;
    r.id = bus.id();
    r.name = bus.name();
    r.is_roundtrip = bus.is_roundtrip();
    for (int i = 0; i < bus.stop_ids_size(); ++i) {
        r.stop_ids.push_back(bus.stop_ids(i));
    }
    return r;
}

CatalogueSaveData::StopToBuses Serializer::DserStopToBuses(const transport_serialize::StopToBuses& stb) {
    CatalogueSaveData::StopToBuses r;
    r.id = stb.id();
    for (int i = 0; i < stb.bus_ids_size(); ++i) {
        r.bus_ids.push_back(stb.bus_ids(i));
    }
    return r;
}

CatalogueSaveData::Distance Serializer::DserDistance(const transport_serialize::Distance& dist) {
    CatalogueSaveData::Distance r;
    r.from = dist.from();
    r.to = dist.to();
    r.distance = dist.distance();
    return r;
}

CatalogueSaveData::GeoDistance Serializer::DserGeoDistance(const transport_serialize::GeoDistance& dist) {
    CatalogueSaveData::GeoDistance r;
    r.from = dist.from();
    r.to = dist.to();
    r.distance = dist.distance();
    return r;
}

CatalogueSaveData::BusToTotal Serializer::DserBusToTotal(const transport_serialize::BusToTotal& dist) {
    CatalogueSaveData::BusToTotal r;
    r.id = dist.id();
    r.distance = dist.distance();
    r.geo_distance = dist.geo_distance();
    return r;
}

} // end namespace transport
