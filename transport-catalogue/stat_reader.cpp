#include "stat_reader.h"

#include <algorithm>
#include <iomanip>

namespace transport {
    
namespace stat {

void StatReader::Read(std::istream& input) {
    int query_count;
    input >> query_count;
    std::string s;
    getline(input, s);
    for(int i=0; i<query_count; ++i) {
        getline(input, s);
        switch(s[0]) {
        case 'B':
            queries_.push_back({QueryType::BUS, move(s.substr(4, s.length()))});
            break;
        case 'S':
            queries_.push_back({QueryType::STOP, move(s.substr(5, s.length()))});
            break;
        }
        
    }
}

void StatReader::ProcessQueries(const TransportCatalogue& catalogue, std::ostream& out) const {
    for(const auto& q: queries_) {
        std::string_view sv = q.data;
        trim(sv);
        switch (q.type) {
        case BUS: {
            const auto& info = catalogue.GetBusInfo(sv);
            if (info.name == "") {
                out << "Bus " << sv << ": not found" << std::endl;
            } else {
                out << "Bus " << sv << ": " << info.totalStopsCount << " stops on route, "
                        << info.uniqueStopsCount << " unique stops, " << info.routeLength
                        << " route length, " << std::setprecision(6) << info.curvature
                        << " curvature" << std::endl;
            }
            break;
        }
        case STOP: {
            const auto& stop_to_buses = catalogue.GetStopToBuses();
            if (stop_to_buses.count(sv) == 0) {
                out << "Stop " << sv << ": not found" << std::endl;
            } else if (stop_to_buses.at(sv).size() == 0) {
                out << "Stop " << sv << ": no buses" << std::endl;
            } else {
                std::vector<std::string_view> stops(stop_to_buses.at(sv).begin(), stop_to_buses.at(sv).end());
                std::sort(stops.begin(), stops.end());
                out << "Stop " << sv << ": buses";
                for (const auto& stop: stops) {
                    out << " " << stop;
                }
                out << std::endl;
            }
            break;
        }
        } // end switch
    }
}

void StatReader::trim(std::string_view& sv) {
    auto pos1 = sv.find_first_not_of(' ');
    auto pos2 = sv.find_last_not_of(' ');
    sv = sv.substr(pos1, pos2 - pos1 + 1);
}

} // end namespace stat

} // end namespace transport
