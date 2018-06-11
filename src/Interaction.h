/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    Interaction.h
    Defines interaction data structures.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
