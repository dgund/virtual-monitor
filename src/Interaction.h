/*
    Interaction.h
    Defines interaction data structures.
*/

#ifndef INTERACTION_H
#define INTERACTION_H

#include <ctime>

#include "Location.h"

namespace virtualMonitor {

// Source: https://developer.apple.com/documentation/uikit/uigesturerecognizer
enum InteractionType {
    InteractionTypeTap,
    InteractionTypePinch,
    InteractionTypeRotation,
    InteractionTypeSwipe,
    InteractionTypePan,
    InteractionTypeEdgePan,
    InteractionTypeLongPress
};

struct Interaction {
    InteractionType type;
    std::time_t time;
    Coord3D *physicalLocation;
    Coord2D *virtualLocation;
};

} /* namespace virtualMonitor */

#endif /* INTERACTION_H */
