#include "domain.h"

namespace transport {

size_t StopHasher::operator()(std::pair<const Stop*, const Stop*> stops) const {
    return std::hash<const void*>{}(stops.first) + 37 * std::hash<const void*>{}(stops.second);
}

size_t StopHasher::operator()(const Stop* stop) const {
    return std::hash<const void*>{}(stop);
}
    
} // end namespace transport