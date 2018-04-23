/*
    CalibrationInteractionHandler.cpp
    Handles calibration interactions with the virtual monitor.
*/

#include "CalibrationInteractionHandler.h"

#include <iostream>

namespace virtualMonitor {

CalibrationInteractionHandler::CalibrationInteractionHandler() : InteractionHandler() {}

int CalibrationInteractionHandler::handleInteractionStartEvent() {
    return 0;
}

int CalibrationInteractionHandler::handleInteractionMoveEvent() {
    return 0;
}

int CalibrationInteractionHandler::handleInteractionEndEvent() {
    return 0;
}

} /* namespace virtualMonitor */
