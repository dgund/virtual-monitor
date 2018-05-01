/*
    InteractionHandler.h
    Handles interactions with the virtual monitor.
*/

#ifndef INTERACTIONHANDLER_H
#define INTERACTIONHANDLER_H

#include "Interaction.h"

#include <sys/time.h>
#include <time.h>
#include <iostream>
#include <fstream>

namespace virtualMonitor {

class HysteresisCounter {
public:
    enum HysteresisValue { A, B };
public:
    int max;
private:
    int count;
    HysteresisValue value;
public:
    HysteresisCounter(int max);
    virtual ~HysteresisCounter() {}
    virtual void reset();
    virtual void updateForValue(HysteresisValue value);
    virtual HysteresisValue getValue() { return this->value; }
};

class InteractionHandler {
protected:
    Coord2D *firstLocation;
    uint32_t firstTimestamp;
    Coord2D *lastLocation;
    uint32_t lastTimestamp;
private:
    HysteresisCounter *interactionCounter;
public:
    InteractionHandler();
    virtual ~InteractionHandler();
    virtual bool handleInteraction(Interaction *interaction);
    virtual void writeTapLocation(int xpos, int ypos);
private:
    virtual int handleInteractionStartEvent();
    virtual int handleInteractionMoveEvent();
    virtual int handleInteractionEndEvent();
    virtual uint32_t timeDifference(uint32_t time1, uint32_t time2);
};

} /* namespace virtualMonitor */

#endif /* INTERACTIONHANDLER_H */
