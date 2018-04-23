/*
    MouseInteractionHandler.h
    Handles mouse interactions with the virtual monitor.
*/

#ifndef MOUSEINTERACTIONHANDLER_H
#define MOUSEINTERACTIONHANDLER_H

#include "InteractionHandler.h"

namespace virtualMonitor {

class MouseInteractionHandler : public InteractionHandler {
public:
    MouseInteractionHandler();

private:
    virtual int handleInteractionStartEvent();
    virtual int handleInteractionMoveEvent();
    virtual int handleInteractionEndEvent();
};

} /* namespace virtualMonitor */

#endif /* MOUSEINTERACTIONHANDLER_H */
