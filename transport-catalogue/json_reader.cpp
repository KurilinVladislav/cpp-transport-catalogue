#include "json_reader.h"

#include <sstream>
#include <algorithm>

namespace transport {
namespace io {
    
JsonReader::JsonReader(TransportCatalogue& db, const RequestHandler& handler, renderer::MapRenderer& renderer)
    : db_(db), handler_(handler), renderer_(renderer) {
}

void JsonReader::ReadJsonFromStream(std::istream& input) {
    requests_ = std::move(json::Document{json::Load(input)});
}
    
void JsonReader::FillDB() {
    const json::Array& base_requests = requests_.GetRoot().AsMap().at("base_requests").AsArray();
    json::Array stop_requests;
    //stop_requests.reserve(base_requests.size());
    json::Array bus_requests;
    //bus_requests.reserve(base_requests.size());
    
    for(auto& request: base_requests) {
        if (request.AsMap().at("type").AsString() == "Stop") {
            stop_requests.emplace_back(std::move(request));
        } else /*if (request.AsMap().at("type").AsString() == "Bus")*/ {
            bus_requests.emplace_back(std::move(request));
        }
    }
    
    for(auto& request: stop_requests) {
        db_.AddStop(request.AsMap().at("name").AsString(),
                    {request.AsMap().at("latitude").AsDouble(),
                     request.AsMap().at("longitude").AsDouble()});
    }
    for(auto& request: stop_requests) {
        for(auto& [to_name, dist]: request.AsMap().at("road_distances").AsMap()) {
            const Stop* from = db_.FindStop(request.AsMap().at("name").AsString());
            const Stop* to = db_.FindStop(to_name);
            db_.SetDistance(from, to, dist.AsInt());
        }
    }
    for(auto& request: bus_requests) {
        bool looped_flag = request.AsMap().at("is_roundtrip").AsBool();
        std::vector<std::string_view> stop_names;
        for(auto& stop: request.AsMap().at("stops").AsArray()) {
            stop_names.push_back(stop.AsString());
        }
        if (looped_flag == false) {
            std::vector<std::string_view> tmp(stop_names.rbegin() + 1, stop_names.rend());
            for(auto& s: tmp) {
                stop_names.push_back(s);
            }
        }
        db_.AddBus(request.AsMap().at("name").AsString(), stop_names, looped_flag);
    }
}

void JsonReader::ProcessAndApplyRenderSettings() {
    auto& s = requests_.GetRoot().AsMap().at("render_settings").AsMap();
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

void JsonReader::ProcessStatRequests(std::ostream& output) {
    json::Array result;
    for(auto& request: requests_.GetRoot().AsMap().at("stat_requests").AsArray()) {
        if (request.AsMap().at("type").AsString() == "Stop") { // Stop info requests
            result.emplace_back(std::move(json::Node{ProcessStopInfoRequest(request.AsMap())}));
        } else if (request.AsMap().at("type").AsString() == "Bus") {
            result.emplace_back(std::move(json::Node{ProcessBusInfoRequest(request.AsMap())}));
        } else {
            result.emplace_back(std::move(json::Node{ProcessMapRequest(request.AsMap())}));
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
            return json::Dict{
                {"request_id", id},
                {"buses", json::Array{}}
            };
        }
        std::vector<std::string_view> sorted_buses(res->begin(), res->end());
        std::sort(sorted_buses.begin(), sorted_buses.end());
        return json::Dict{
            {"request_id", id},
            {"buses", json::Array(sorted_buses.begin(), sorted_buses.end())}
        };
    } else {
        return json::Dict{
            {"request_id", id},
            {"error_message", "not found"}
        };
    }
}

json::Dict JsonReader::ProcessBusInfoRequest(const json::Dict& request) const {
    int id = request.at("id").AsInt();
    std::string name = "";
    if (request.at("name").IsString() == true) {
        name = request.at("name").AsString();
    }
    auto res = handler_.GetBusStat(name);
    if (res) {
        return json::Dict{
            {"request_id", id},
            {"curvature", res->curvature},
            {"route_length", res->routeLength},
            {"stop_count", res->totalStopsCount},
            {"unique_stop_count", res->uniqueStopsCount}
        };
    } else {
        return json::Dict{
            {"request_id", id},
            {"error_message", "not found"}
        };
    }
}
    
json::Dict JsonReader::ProcessMapRequest(const json::Dict& request) const {
    int id = request.at("id").AsInt();
    auto res = handler_.RenderMap();
    std::ostringstream out;
    res.Render(out);
    return json::Dict{
        {"request_id", id},
        {"map", out.str()}
    };
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
