#define _USE_MATH_DEFINES
#include "geo.h"

namespace transport {
namespace geo {
    
bool Coordinates::operator==(const Coordinates& other) const {
        return lat == other.lat && lng == other.lng;
    }

bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}

} // end namespace geo
} // end namespace transport
