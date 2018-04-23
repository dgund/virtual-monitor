/*
    MouseInteractionHandler.cpp
    Handles mouse interactions with the virtual monitor.
*/

#include "MouseInteractionHandler.h"

#include <iostream>

namespace virtualMonitor {

MouseInteractionHandler::MouseInteractionHandler() : InteractionHandler() {}

int MouseInteractionHandler::handleInteractionStartEvent() {
    return 0;
}

int MouseInteractionHandler::handleInteractionMoveEvent() {
    return 0;
}

int MouseInteractionHandler::handleInteractionEndEvent() {
    return 0;
}

} /* namespace virtualMonitor */
