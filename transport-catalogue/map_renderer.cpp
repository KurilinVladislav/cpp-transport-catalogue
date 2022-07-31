#include "map_renderer.h"

#include <unordered_set>

namespace transport {
namespace renderer {
    
bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
    return {
        (coords.lng - min_lon_) * zoom_coeff_ + padding_,
        (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
}
    
void MapRenderer::ApplySettings(const RenderSettings& settings) {
    render_settings_ = std::move(settings);
}

const RenderSettings& MapRenderer::GetSettings() const {
    return render_settings_;
}
    
svg::Document MapRenderer::RenderMap(const std::vector<const Bus*>& buses) const {
    std::unordered_set<const Stop*, StopHasher> stops;
    for(const auto& bus: buses) {
        if (bus->is_roundtrip == false) {
            for (int i=0; i<static_cast<int>(bus->stops.size() - 1 / 2); ++i) {
                stops.insert(bus->stops[i]);
            }
        } else {
            for(const auto& stop: bus->stops) {
                stops.insert(stop);
            }
        }
    }
    std::vector<const Stop*> sorted_stops(stops.begin(), stops.end());
    std::sort(sorted_stops.begin(), sorted_stops.end(),
             [](const auto& lhs, const auto& rhs){
                 return lhs->name < rhs->name;
             });
    std::vector<geo::Coordinates> points;
    for(const auto& stop: stops) {
        points.push_back(geo::Coordinates{stop->coordinates});
    }
    
    SphereProjector projector(points.begin(), points.end(), render_settings_.width, render_settings_.height, render_settings_.padding);

    svg::Document doc;
    RenderBuses(buses, doc, projector);
    RenderBusNames(buses, doc, projector);
    RenderStops(sorted_stops, doc, projector);
    RenderStopNames(sorted_stops, doc, projector);

    return doc;
}
    
void MapRenderer::RenderBuses(const std::vector<const Bus*>& buses, svg::Document& doc, const SphereProjector& projector) const {
    int color_index = 0;
    int palette_size = static_cast<int>(render_settings_.color_palette.size());
    for(const auto& bus: buses) {
        svg::Polyline line;
        line.SetFillColor(svg::NoneColor)
            .SetStrokeColor(render_settings_.color_palette[color_index % palette_size])
            .SetStrokeWidth(render_settings_.line_width)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        for(const auto& stop: bus->stops) {
            line.AddPoint(projector(stop->coordinates));
        }
        doc.Add(line);
        ++color_index;
    }
}

void MapRenderer::RenderBusNames(const std::vector<const Bus*>& buses, svg::Document& doc, const SphereProjector& projector) const {
    int color_index = 0;
    int palette_size = static_cast<int>(render_settings_.color_palette.size());
    for(const auto& bus: buses) {
        svg::Text text;
        text.SetPosition(projector(bus->stops[0]->coordinates))
            .SetOffset(render_settings_.bus_label_offset)
            .SetFontSize(render_settings_.bus_label_font_size)
            .SetFontFamily("Verdana")
            .SetFontWeight("bold")
            .SetData(bus->name);
        svg::Text underlayer{text};
        underlayer.SetFillColor(render_settings_.underlayer_color)
                  .SetStrokeColor(render_settings_.underlayer_color)
                  .SetStrokeWidth(render_settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetFillColor(render_settings_.color_palette[color_index % palette_size]);
        doc.Add(underlayer);
        doc.Add(text);
        if (bus->is_roundtrip == false && bus->stops[(bus->stops.size() - 1) / 2] != bus->stops[0]) {
            svg::Text text2{text};
            svg::Text underlayer2{underlayer};
            svg::Point pos{projector(bus->stops[(bus->stops.size() - 1) / 2]->coordinates)};
            text2.SetPosition(pos);
            underlayer2.SetPosition(pos);
            doc.Add(underlayer2);
            doc.Add(text2);
        }
        ++color_index;
    }
}

void MapRenderer::RenderStops(const std::vector<const Stop*>& stops, svg::Document& doc, const SphereProjector& projector) const {
    for (const auto& stop: stops) {
        svg::Circle circle;
        circle.SetCenter(projector(stop->coordinates))
            .SetRadius(render_settings_.stop_radius)
            .SetFillColor("white");
        doc.Add(circle);
    }
}

void MapRenderer::RenderStopNames(const std::vector<const Stop*>& stops, svg::Document& doc, const SphereProjector& projector) const {
    for (const auto& stop: stops) {
        svg::Text text;
        text.SetPosition(projector(stop->coordinates))
            .SetOffset(render_settings_.stop_label_offset)
            .SetFontSize(render_settings_.stop_label_font_size)
            .SetFontFamily("Verdana")
            .SetData(stop->name);
        svg::Text underlayer{text};
        underlayer.SetFillColor(render_settings_.underlayer_color)
                  .SetStrokeColor(render_settings_.underlayer_color)
                  .SetStrokeWidth(render_settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetFillColor("black");
        doc.Add(underlayer);
        doc.Add(text);
    }
}

} // end namespace renderer
} // end namespace transport
