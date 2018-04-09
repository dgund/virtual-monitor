/*
    Interaction.h
    Defines interaction data structures.
*/

#ifndef INTERACTION_H
#define INTERACTION_H

#include <cstdint>

#include "Location.h"

namespace virtualMonitor {

// Source: https://developer.apple.com/documentation/uikit/uigesturerecognizer
enum InteractionType {
    Tap,
    Pinch,
    Rotation,
    Swipe,
    Pan,
    EdgePan,
    LongPress
};

struct Interaction {
    InteractionType type;
    uint32_t time;
    Coord3D *physicalLocation;
    Coord2D *virtualLocation;
    float surfaceRegressionA;
    float surfaceRegressionB;
};

} /* namespace virtualMonitor */

#endif /* INTERACTION_H */
