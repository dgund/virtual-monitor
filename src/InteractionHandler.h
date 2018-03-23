/*
    InteractionHandler.h
    Handles interactions with the virtual monitor.
*/

#ifndef INTERACTIONHANDLER_H
#define INTERACTIONHANDLER_H

#include "Interaction.h"

namespace virtualMonitor {

class InteractionHandler {
    public:
        InteractionHandler();
        virtual ~InteractionHandler();

        virtual int handleInteraction(Interaction *interaction);
};

} /* namespace virtualMonitor */

#endif /* INTERACTIONHANDLER_H */
