#include "svg.h"
#include <iomanip>

namespace svg {

using namespace std::literals;
    
std::ostream& ColorPrinter::operator()(std::monostate) const {
    out << "none"sv;
    return out;
}
    
std::ostream& ColorPrinter::operator()(std::string_view s) const {
    out << s;
    return out;
}
    
std::ostream& ColorPrinter::operator()(Rgb val) const {
    out << "rgb("sv << static_cast<uint16_t>(val.red) << ","sv << static_cast<uint16_t>(val.green) << ","sv << static_cast<uint16_t>(val.blue) << ")"sv;
    return out;
}
    
std::ostream& ColorPrinter::operator()(Rgba val) const {
    out << "rgba("sv << static_cast<uint16_t>(val.red) << ","sv << static_cast<uint16_t>(val.green) << ","sv << static_cast<uint16_t>(val.blue) << "," << val.opacity << ")"sv;
    return out;
}
    
std::ostream& operator<<(std::ostream& out, const Color& color) {
    return std::visit(ColorPrinter{out}, color);
}
    
std::ostream& operator<<(std::ostream& out, const StrokeLineCap value) {
    switch(value) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}
    
std::ostream& operator<<(std::ostream& out, const StrokeLineJoin value) {
    switch(value) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
    }
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// ---------- Polyline --------------
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    int i=0;
    for (auto& p: points_) {
        if (i!=0) {
            out << " "sv;
        }
        out << p.x << ","sv << p.y;
        ++i;
    }
    out << "\""sv;
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << "/>"sv;
}
    
// -------------- Text ---------------

Text& Text::SetPosition(Point pos) {
    pos_.x = pos.x;
    pos_.y = pos.y;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_.x = offset.x;
    offset_.y = offset.y;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    std::stringstream ss;
    ss << size;
    ss >> size_;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    
    out << "font-size=\""sv << size_ << "\""sv;
    if (font_family_ != "") {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (font_weight_ != "") {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    // Выводим атрибуты, унаследованные от PathProps
    RenderAttrs(context.out);
    out << ">"sv;
    
    std::string_view sv(data_);
    auto begin = sv.find_first_not_of(' ');
    auto end = sv.find_last_not_of(' ');
    if (begin != std::string_view::npos) {
        sv = sv.substr(begin, end - begin + 1);
        for(const char& c: sv) {
            if (c == '\"') {
                out << "&quot;"sv;
                continue;
            }
            if (c == '<') {
                out << "&lt;"sv;
                continue;
            }
            if (c == '>') {
                out << "&gt;"sv;
                continue;
            }
            if (c == '\'') {
                out << "&apos;"sv;
                continue;
            }
            if (c == '&') {
                out << "&amp;"sv;
                continue;
            } else {
                out << c;
            }
        }
    }
    out << "</text>"sv;
}
    
// ----------- Document ------------

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx(out, 2, 2);
    for(auto& obj: objects_) {
        obj->Render(ctx);
    }
    out << "</svg>"sv;
}


}  // namespace svg
