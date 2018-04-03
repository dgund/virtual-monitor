/*
    Location.h
    Defines location data structures.
*/

#ifndef LOCATION_H
#define LOCATION_H

namespace virtualMonitor {

struct Coord2D {
    int x;
    int y;
};

struct Coord3D {
    int x;
    int y;
    float z;
};

} /* namespace virtualMonitor */

#endif /* LOCATION_H */
