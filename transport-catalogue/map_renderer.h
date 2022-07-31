#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace transport {
namespace renderer {

inline const double EPSILON = 1e-6;
bool IsZero(double value);
    
struct RenderSettings {
    double width = 0;
    double height = 0;
    double padding = 0;
    double line_width = 0;
    double stop_radius = 0;
    int bus_label_font_size = 0;
    svg::Point bus_label_offset = {0, 0};
    int stop_label_font_size = 0;
    svg::Point stop_label_offset = {0, 0};
    svg::Color underlayer_color = "white";
    double underlayer_width = 0;
    std::vector<svg::Color> color_palette = {};
};

class SphereProjector {
public:
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding);

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


class MapRenderer {
public:
    MapRenderer() = default;
    void ApplySettings(const RenderSettings& settings);
    const RenderSettings& GetSettings() const;
    svg::Document RenderMap(const std::vector<const Bus*>& buses) const;
private:
    void RenderBuses(const std::vector<const Bus*>& buses, svg::Document& doc, const SphereProjector& projector) const;
    void RenderBusNames(const std::vector<const Bus*>& buses, svg::Document& doc, const SphereProjector& projector) const;
    void RenderStops(const std::vector<const Stop*>& stops, svg::Document& doc, const SphereProjector& projector) const;
    void RenderStopNames(const std::vector<const Stop*>& stops, svg::Document& doc, const SphereProjector& projector) const;
    
    RenderSettings render_settings_ = {
        1200.0,
        1200.0,
        50.0,
        14.0,
        5.0,
        20,
        svg::Point{7.0, 15.0},
        20,
        svg::Point{7.0, -3.0},
        svg::Color{svg::Rgba{255, 255, 255, 0.85}},
        3.0,
        {
            svg::Color{"green"},
            svg::Color{svg::Rgb{255, 160, 0}},
            svg::Color{"red"}
        }
    };
};
    

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding)
    : padding_(padding) {
    // Если точки поверхности сферы не заданы, вычислять нечего
    if (points_begin == points_end) {
        return;
    }

    // Находим точки с минимальной и максимальной долготой
    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    // Находим точки с минимальной и максимальной широтой
    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    // Вычисляем коэффициент масштабирования вдоль координаты x
    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_)) {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    // Вычисляем коэффициент масштабирования вдоль координаты y
    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat)) {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom) {
        // Коэффициенты масштабирования по ширине и высоте ненулевые,
        // берём минимальный из них
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    } else if (width_zoom) {
        // Коэффициент масштабирования по ширине ненулевой, используем его
        zoom_coeff_ = *width_zoom;
    } else if (height_zoom) {
        // Коэффициент масштабирования по высоте ненулевой, используем его
        zoom_coeff_ = *height_zoom;
    }
}
    


} // end namespace renderer
} // end namespace transport
