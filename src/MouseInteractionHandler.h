/*
    MouseInteractionHandler.h
    Handles mouse interactions with the virtual monitor.
*/

#ifndef MOUSEINTERACTIONHANDLER_H
#define MOUSEINTERACTIONHANDLER_H

#include "InteractionHandler.h"
#include "MouseController.h"

namespace virtualMonitor {

class MouseInteractionHandler : public InteractionHandler {
private:
    MouseController mouseController;
public:
    MouseInteractionHandler();

private:
    virtual int handleInteractionStartEvent();
    virtual int handleInteractionMoveEvent();
    virtual int handleInteractionEndEvent();
};

} /* namespace virtualMonitor */

#endif /* MOUSEINTERACTIONHANDLER_H */
