#include "pixel.h"
#include <iomanip>

std::ostream &operator <<(std::ostream &to, const Pixel &px) {
    std::streamsize old = to.precision(2);
    to << "r=" << px.r;
    to << ", g=" << px.g;
    to << ", b=" << px.b;
    to.precision(old);
    return to;
}
