/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    InteractionHandler.h
    Handles interactions with the virtual monitor.

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
    int maxA, maxB;
private:
    int count;
    HysteresisValue value;
public:
    HysteresisCounter(int maxA, int maxB);
    virtual ~HysteresisCounter() {}
    virtual void reset();
    virtual void updateForValue(HysteresisValue value);
    virtual HysteresisValue getValue() { return this->value; }
    virtual int maxForValue(HysteresisValue value);
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
