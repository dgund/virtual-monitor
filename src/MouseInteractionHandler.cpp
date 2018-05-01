/*
    MouseInteractionHandler.cpp
    Handles mouse interactions with the virtual monitor.
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
