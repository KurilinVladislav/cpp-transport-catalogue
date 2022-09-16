#include "json_reader.h"
#include "json_builder.h"

#include <sstream>
#include <algorithm>

namespace transport {
namespace io {
    
JsonReader::JsonReader(TransportCatalogue& db, const RequestHandler& handler, renderer::MapRenderer& renderer, TransportRouter& router)
    : db_(db), handler_(handler), renderer_(renderer), router_(router) {
}

void JsonReader::ReadJsonFromStream(std::istream& input) {
    requests_ = std::move(json::Document{json::Load(input)});
}
    
void JsonReader::FillDB() {
    if (requests_.GetRoot().AsDict().count("base_requests") == 0) {
        return;
    }
    const json::Array& base_requests = requests_.GetRoot().AsDict().at("base_requests").AsArray();
    json::Array stop_requests;
    json::Array bus_requests;
    
    for(auto& request: base_requests) {
        if (request.AsDict().at("type").AsString() == "Stop") {
            stop_requests.emplace_back(std::move(request));
        } else /*if (request.AsDict().at("type").AsString() == "Bus")*/ {
            bus_requests.emplace_back(std::move(request));
        }
    }
    
    for(auto& request: stop_requests) {
        db_.AddStop(request.AsDict().at("name").AsString(),
                    {request.AsDict().at("latitude").AsDouble(),
                     request.AsDict().at("longitude").AsDouble()});
    }
    for(auto& request: stop_requests) {
        for(auto& [to_name, dist]: request.AsDict().at("road_distances").AsDict()) {
            const Stop* from = db_.FindStop(request.AsDict().at("name").AsString());
            const Stop* to = db_.FindStop(to_name);
            db_.SetDistance(from, to, dist.AsInt());
        }
    }
    for(auto& request: bus_requests) {
        bool looped_flag = request.AsDict().at("is_roundtrip").AsBool();
        std::vector<std::string_view> stop_names;
        for(auto& stop: request.AsDict().at("stops").AsArray()) {
            stop_names.push_back(stop.AsString());
        }
        if (looped_flag == false) {
            std::vector<std::string_view> tmp(stop_names.rbegin() + 1, stop_names.rend());
            for(auto& s: tmp) {
                stop_names.push_back(s);
            }
        }
        db_.AddBus(request.AsDict().at("name").AsString(), stop_names, looped_flag);
    }
}

void JsonReader::ProcessAndApplyRenderSettings() {
    if (requests_.GetRoot().AsDict().count("render_settings") == 0) {
        return;
    }
    auto& s = requests_.GetRoot().AsDict().at("render_settings").AsDict();
    double width = s.at("width").AsDouble();
    double height = s.at("height").AsDouble();
    double padding = s.at("padding").AsDouble();
    double line_width = s.at("line_width").AsDouble();
    double stop_radius = s.at("stop_radius").AsDouble();
    int bus_label_font_size = s.at("bus_label_font_size").AsInt();
    const json::Array& bus_label_offset = s.at("bus_label_offset").AsArray();
    int stop_label_font_size = s.at("stop_label_font_size").AsInt();
    const json::Array& stop_label_offset = s.at("stop_label_offset").AsArray();
    svg::Color underlayer_color{GetColorFromJsonNode(s.at("underlayer_color"))};
    double underlayer_width = s.at("underlayer_width").AsDouble();
    std::vector<svg::Color> color_palette;
    const json::Array& colors = s.at("color_palette").AsArray();
    for(auto& val: colors) {
        color_palette.push_back(GetColorFromJsonNode(val));
    }
    
    renderer_.ApplySettings({
        width, height, padding, line_width, stop_radius, bus_label_font_size,
        {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()},
        stop_label_font_size,
        {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()},
        underlayer_color, underlayer_width, color_palette
    });
}
    
void JsonReader::ProcessAndApplyRouterSettings() {
    if (requests_.GetRoot().AsDict().count("routing_settings") == 0) {
        return;
    }
    auto& s = requests_.GetRoot().AsDict().at("routing_settings").AsDict();
    size_t bus_wait_time = s.at("bus_wait_time").AsInt();
    double bus_velocity = s.at("bus_velocity").AsDouble();
    router_.ApplySettings({bus_wait_time, bus_velocity});
}

json::Dict JsonReader::ProcessSerializationSettings() const {
    if (requests_.GetRoot().AsDict().count("serialization_settings") == 0) {
        return json::Builder{}.StartDict().Key("file").Value("").EndDict().Build().AsDict();
    }
    const auto& s = requests_.GetRoot().AsDict().at("serialization_settings").AsDict();
    std::string file = s.at("file").AsString();
    return json::Builder{}
            .StartDict()
                .Key("file").Value(file)
            .EndDict().Build().AsDict();
}

void JsonReader::ProcessStatRequests(std::ostream& output) {
    if (requests_.GetRoot().AsDict().count("stat_requests") == 0) {
        return;
    }
    json::Array result;
    for(auto& request: requests_.GetRoot().AsDict().at("stat_requests").AsArray()) {
        if (request.AsDict().at("type").AsString() == "Stop") { // Stop info requests
            result.emplace_back(std::move(json::Node{ProcessStopInfoRequest(request.AsDict())}));
        } else if (request.AsDict().at("type").AsString() == "Bus") {
            result.emplace_back(std::move(json::Node{ProcessBusInfoRequest(request.AsDict())}));
        } else if (request.AsDict().at("type").AsString() == "Map") {
            result.emplace_back(std::move(json::Node{ProcessMapRequest(request.AsDict())}));
        } else /*if (request_.AsDict().at("type").AsString() == "Route")*/ {
            result.emplace_back(std::move(json::Node{ProcessRouteRequest(request.AsDict())}));
        }
    }
    json::Print(json::Document{result}, output);
}
    
json::Dict JsonReader::ProcessStopInfoRequest(const json::Dict& request) const {
    int id = request.at("id").AsInt();
    std::string name = "";
    if (request.at("name").IsString() == true) {
        name = request.at("name").AsString();
    }
    auto res = handler_.GetBusesByStop(name);
    if (res) {
        if (res->size() == 0) {
            json::Array empty{};
            return json::Builder{}
            .StartDict()
                .Key("request_id").Value(id)
                .Key("buses").StartArray().EndArray() // Value(json::Array{})
            .EndDict().Build().AsDict();
        }
        std::vector<std::string_view> sorted_buses(res->begin(), res->end());
        std::sort(sorted_buses.begin(), sorted_buses.end());
        json::Array sorted_buses_array;
        for(std::string_view sv: sorted_buses) {
            json::Node tmp{static_cast<std::string>(sv)};
            sorted_buses_array.emplace_back(std::move(tmp));
        }
        return json::Builder{}
        .StartDict()
            .Key("request_id").Value(id)
            .Key("buses").Value(sorted_buses_array)
        .EndDict().Build().AsDict();
    } else {
        return json::Builder{}
        .StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value("not found")
        .EndDict().Build().AsDict();
    }
}

json::Dict JsonReader::ProcessBusInfoRequest(const json::Dict& request) const {
    int id = request.at("id").AsInt();
    std::string name = "";
    if (request.at("name").IsString() == true) {
        name = request.at("name").AsString();
    }
    if (name == "") {
        return json::Builder{}
        .StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value("not found")
        .EndDict().Build().AsDict();
    }
    auto res = handler_.GetBusStat(name);
    if (res) {
        return json::Builder{}
        .StartDict()
            .Key("request_id").Value(id)
            .Key("curvature").Value(res->curvature)
            .Key("route_length").Value(res->routeLength)
            .Key("stop_count").Value(res->totalStopsCount)
            .Key("unique_stop_count").Value(res->uniqueStopsCount)
        .EndDict().Build().AsDict();
    } else {
        return json::Builder{}
        .StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value("not found")
        .EndDict().Build().AsDict();
    }
}
    
json::Dict JsonReader::ProcessMapRequest(const json::Dict& request) const {
    int id = request.at("id").AsInt();
    auto res = handler_.RenderMap();
    std::ostringstream out;
    res.Render(out);
    return json::Builder{}
    .StartDict()
        .Key("request_id").Value(id)
        .Key("map").Value(out.str())
    .EndDict().Build().AsDict();
}
    
json::Dict JsonReader::ProcessRouteRequest(const json::Dict& request) const {
    int id = request.at("id").AsInt();
    auto res = router_.BuildRoute(request.at("from").AsString(), request.at("to").AsString());
    if (res) {
        // unpack result; if there are no items, out items = [] and time = 0
        json::Array items;
        for(size_t edge_id: res->edges) {
            const auto& edge = router_.GetEdge(edge_id);
            items.emplace_back(std::move(json::Builder{}.StartDict()
                                        .Key("type").Value("Wait")
                                        .Key("stop_name").Value(db_.GetStopById(edge.from)->name)
                                        .Key("time").Value(router_.GetBusWaitTime())
                                        .EndDict().Build().AsDict()));
            std::string bus_name = db_.GetBusById(edge.bus_id)->name;
            items.emplace_back(std::move(json::Builder{}.StartDict()
                                        .Key("type").Value("Bus")
                                        .Key("bus").Value(std::string(bus_name))
                                        .Key("span_count").Value(static_cast<int>(edge.stop_count))
                                        .Key("time").Value(edge.weight - router_.GetBusWaitTime())
                                        .EndDict().Build().AsDict()));
        }
        
        return json::Builder{}.StartDict()
            .Key("request_id").Value(id)
            .Key("total_time").Value(res->weight)
            .Key("items").Value(items)
            .EndDict().Build().AsDict();
    } else {
        return json::Builder{}
            .StartDict()
                .Key("request_id").Value(id)
                .Key("error_message").Value("not found")
            .EndDict().Build().AsDict();
    }
}
    
svg::Color JsonReader::GetColorFromJsonNode(const json::Node& node) const {
    if (node.IsString()) {
        return svg::Color{node.AsString()};
    } else /*if (node.IsArray())*/ {
        uint8_t red = node.AsArray()[0].AsInt();
        uint8_t green = node.AsArray()[1].AsInt();
        uint8_t blue = node.AsArray()[2].AsInt();
        if (node.AsArray().size() == 4) {
            return svg::Color{svg::Rgba{red, green, blue, node.AsArray()[3].AsDouble()}};
        } else {
            return svg::Color{svg::Rgb{red, green, blue}};
        }
    }
}

    
} // end namespace io
} // end namespace transport
