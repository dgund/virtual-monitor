/*
    InteractionHandler.cpp
    Handles interactions with the virtual monitor.
*/

#include "InteractionHandler.h"

#include <algorithm>
#include <iostream>
#include <limits>

namespace virtualMonitor {

#define INTERACTION_TIME_DIFFERENCE_MAX 30

#define HYSTERESIS_MAX 5

HysteresisCounter::HysteresisCounter(int max) {
    this->max = max;
    this->reset();
}

void HysteresisCounter::reset() {
    this->count = 0;
    this->value = HysteresisValue::A;
}

void HysteresisCounter::updateForValue(HysteresisValue value) {
    if (value == this->value) {
        this->count = std::min(this->count + 1, this->max);
    } else {
        this->count--;
        if (this->count <= 0) {
            this->value = value;
            this->count = this->max;
        }
    }
}

InteractionHandler::InteractionHandler() {
    this->interactionCounter = new HysteresisCounter(HYSTERESIS_MAX);
    this->firstLocation = new Coord2D();
    this->firstLocation->x = -1;
    this->firstLocation->y = -1;
    this->firstTimestamp = 0;
    this->lastLocation = new Coord2D();
    this->lastLocation->x = -1;
    this->lastLocation->y = -1;
    this->lastTimestamp = 0;
}

InteractionHandler::~InteractionHandler() {
    delete this->interactionCounter;
    delete this->firstLocation;
    delete this->lastLocation;
}

int InteractionHandler::handleInteraction(Interaction *interaction) {
    std::cout << "InteractionHandler: Interaction at x = " << interaction->physicalLocation->x << ", y = " << interaction->physicalLocation->y << ", depth = " << interaction->physicalLocation->z << std::endl;

    if (interaction == NULL || interaction->virtualLocation == NULL) {
        return -1;
    }

    // Coordinates for a real interaction provided
    bool isInteraction = true;

    bool wasOngoingInteraction = (this->interactionCounter->getValue() == HysteresisCounter::HysteresisValue::A);
    this->interactionCounter->updateForValue(isInteraction ? HysteresisCounter::HysteresisValue::A : HysteresisCounter::HysteresisValue::B);
    bool isOngoingInteraction = (this->interactionCounter->getValue() == HysteresisCounter::HysteresisValue::A);

    // If there is an interaction, move the mouse to the virtual location
    if (isOngoingInteraction) {
        this->lastLocation->x = interaction->virtualLocation->x;
        this->lastLocation->y = interaction->virtualLocation->y;
        this->lastTimestamp = interaction->time;

        this->handleInteractionMoveEvent();

        // If this is a new interaction, click the mouse down
        if (!wasOngoingInteraction) {
            this->firstLocation->x = interaction->virtualLocation->x;
            this->firstLocation->y = interaction->virtualLocation->y;
            this->firstTimestamp = interaction->time;

            this->handleInteractionStartEvent();
        }
    }

    // If there was an interaction that just ended, click the mouse up
    else if (wasOngoingInteraction) {
        this->handleInteractionEndEvent();
        // TODO Possibly simulate click
    }

    return 0;
}

int InteractionHandler::handleInteractionStartEvent() {
    return 0;
}

int InteractionHandler::handleInteractionMoveEvent() {
    return 0;
}

int InteractionHandler::handleInteractionEndEvent() {
    return 0;
}

uint32_t InteractionHandler::timeDifference(uint32_t time1, uint32_t time2) {
    // Assumes that time2 is later than time 1
    // If time1 <= time2, this is normal
    if (time1 <= time2) {
        return time2 - time1;
    }
    // Otherwise, time2 must have overflowed and wrapped around
    else {
        return std::numeric_limits<uint32_t>::max() - time1 + time2;
    }
}

} /* namespace virtualMonitor */
