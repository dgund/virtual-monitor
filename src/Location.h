/*
    Location.h
    Defines location data structures.
*/

#ifndef LOCATION_H
#define LOCATION_H

namespace virtualMonitor {

struct VirtualLocation {
    unsigned long x;
    unsigned long y;
};

struct PhysicalLocation {
    unsigned long x;
    unsigned long y;
    unsigned long z;
};

} /* namespace virtualMonitor */

#endif /* LOCATION_H */
