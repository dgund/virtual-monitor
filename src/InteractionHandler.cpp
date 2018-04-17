/*
    InteractionHandler.cpp
    Handles interactions with the virtual monitor.
*/

#include "InteractionHandler.h"

#include <unistd.h>
#include <iostream>

namespace virtualMonitor {

InteractionHandler::InteractionHandler() {

}

InteractionHandler::~InteractionHandler() {

}

int InteractionHandler::handleInteraction(Interaction *interaction) {
    std::cout << "InteractionHandler: Interaction at x = " << interaction->physicalLocation->x << ", y = " << interaction->physicalLocation->y << ", depth = " << interaction->physicalLocation->z << std::endl;
    return 0;
}

} /* namespace virtualMonitor */
