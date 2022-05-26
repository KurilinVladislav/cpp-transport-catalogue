#include "input_reader.h"

#include <sstream>
#include <algorithm>

namespace transport {
    
namespace input {

void InputReader::Read(std::istream& input) {
    int query_count;
    input >> query_count;
    std::string s;
    getline(input, s);
    for(int i=0; i<query_count; ++i) {
        getline(input, s);
        if (s[0] == 'S') { // Stop X: ...
            stop_queries_.push_back(move(s.substr(5, s.length() - 5))); // skip 'Stop '
        } else if (s[0] == 'B') { // Bus X: ...
            bus_queries_.push_back(move(s.substr(4, s.length() - 4))); // skip 'Bus '
        }
    }
}

std::vector<StopQuery> InputReader::ProcessStopQueries() const {
    std::vector<StopQuery> result;
    for(const std::string& s: stop_queries_) {
        std::string_view sv = s;
        auto separator_pos = sv.find(':');
        std::string_view name = sv.substr(0, separator_pos);
        trim(name);
        sv.remove_prefix(separator_pos + 1); // skip to after ':'
        separator_pos = sv.find(',');
        std::string_view lat = sv.substr(0, separator_pos);
        trim(lat);
        sv.remove_prefix(separator_pos + 1);
        separator_pos = sv.find(',');
        std::string_view lng = sv.substr(0, separator_pos);
        trim(lng);
        sv.remove_prefix(separator_pos + 1); // after 2nd ','
        
        std::stringstream ss;
        std::unordered_map<std::string_view, int> distances = {};
        if (separator_pos != std::string_view::npos) { // if there is data after coordinates
            while (sv.size() > 0) {
                separator_pos = sv.find(',');
                auto right = separator_pos == std::string_view::npos? sv.length() : separator_pos;
                std::string_view tmp = sv.substr(0, right); // '   100m   to   Stop   X   '
                trim(tmp); // == '100m   to   Stop   X'
                auto tmp_pos = tmp.find('m');
                ss << tmp.substr(0, tmp_pos) << std::endl; // '100'
                int dist;
                ss >> dist;
                tmp_pos = tmp.find_first_not_of(' ', tmp.find("to") + 2); // 'Stop   X'
                std::string_view destination_name = tmp.substr(tmp_pos, tmp.length() - tmp_pos);
                distances[destination_name] = dist;
                sv.remove_prefix(std::min(right + 1, sv.length()));
            }
        }
        
        ss << lat << "\n" << lng;
        double lat_d, lng_d;
        ss >> lat_d >> lng_d;
        
        Coordinates coord{lat_d, lng_d};
        result.push_back({name, coord, distances});
    }
    return result;
}

std::vector<BusQuery> InputReader::ProcessBusQueries() const {
    std::vector<BusQuery> result;
    for(const std::string& s: bus_queries_) {
        std::string_view sv = s;
        auto separator_pos = sv.find(':');
        std::string_view name = sv.substr(0, separator_pos);
        trim(name);
        sv.remove_prefix(separator_pos + 1); // skip ':'

        bool looped_flag = true;
        char separator_char = '>';
        separator_pos = sv.find('>');
        if (separator_pos == std::string_view::npos) {
            separator_pos = sv.find('-'); // guaranteed to be here instead of > then
            looped_flag = false;
            separator_char = '-';
        }

        std::vector<std::string_view> stop_names;
        while (separator_pos != std::string_view::npos) {
            std::string_view tmp = sv.substr(0, separator_pos);
            trim(tmp);
            stop_names.push_back(tmp);
            sv.remove_prefix(separator_pos + 1);
            separator_pos = sv.find(separator_char);
        }
        trim(sv);
        stop_names.push_back(sv); // remaining one

        if (looped_flag == false) {
            std::vector<std::string_view> tmp(stop_names.rbegin() + 1, stop_names.rend());
            for(auto& s: tmp) {
                stop_names.push_back(s);
            }
        }
        result.push_back({name, stop_names});
    }
    return result;
}

void InputReader::trim(std::string_view& sv) {
    auto pos1 = sv.find_first_not_of(' ');
    auto pos2 = sv.find_last_not_of(' ');
    sv = sv.substr(pos1, pos2 - pos1 + 1);
}

/*
void InputReader::Print() const {
    std::cout << "Stop queries:" << std::endl;
    for(const auto& s: stop_queries_) {
        std::cout << s << std::endl;
    }
    std::cout << "Bus queries:" << std::endl;
    for(const auto& s: bus_queries_) {
        std::cout << s << std::endl;
    }
}*/

} // end namespace input

} // end namespace transport
