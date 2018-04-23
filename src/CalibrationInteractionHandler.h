/*
    CalibrationInteractionHandler.h
    Handles calibration interactions with the virtual monitor.
*/

#ifndef CALIBRATIONINTERACTIONHANDLER_H
#define CALIBRATIONINTERACTIONHANDLER_H

#include "InteractionHandler.h"

namespace virtualMonitor {

class CalibrationInteractionHandler : public InteractionHandler {
public:
    CalibrationInteractionHandler();

private:
    virtual int handleInteractionStartEvent();
    virtual int handleInteractionMoveEvent();
    virtual int handleInteractionEndEvent();
};

} /* namespace virtualMonitor */

#endif /* CALIBRATIONINTERACTIONHANDLER_H */
