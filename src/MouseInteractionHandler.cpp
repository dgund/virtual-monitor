/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    MouseInteractionHandler.cpp
    Handles mouse interactions with the virtual monitor.

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

#include "MouseInteractionHandler.h"
#include <iostream>

namespace virtualMonitor {

MouseInteractionHandler::MouseInteractionHandler() : InteractionHandler() {}

int MouseInteractionHandler::handleInteractionStartEvent() {
    mouseController.leftMouseDown(this->lastLocation);
    return 0;
}

int MouseInteractionHandler::handleInteractionMoveEvent() {
    mouseController.move(this->lastLocation);
    return 0;
}

int MouseInteractionHandler::handleInteractionEndEvent() {
    mouseController.leftMouseUp(this->lastLocation);
    // write to output file
    writeTapLocation(this->lastLocation->x, this->lastLocation->y);
    return 0;
}

} /* namespace virtualMonitor */
