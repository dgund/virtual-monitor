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

#define NOINTERACTION_COUNT_MAX 10
#define INTERACTION_COUNT_MAX 2

#define OUTPUT_FILE "outputFile.csv"

HysteresisCounter::HysteresisCounter(int maxA, int maxB) {
    this->maxA = maxA;
    this->maxB = maxB;
    this->reset();
}

void HysteresisCounter::reset() {
    HysteresisValue resetValue = HysteresisValue::A;
    this->value = resetValue;
    this->count = maxForValue(resetValue);
}

void HysteresisCounter::updateForValue(HysteresisValue value) {
    if (value == this->value) {
        this->count = std::min(this->count + 1, maxForValue(value));
    } else {
        this->count--;
        if (this->count <= 0) {
            this->value = value;
            this->count = maxForValue(value);
        }
    }
}

int HysteresisCounter::maxForValue(HysteresisValue value) {
    if (value == HysteresisValue::A) {
        return this->maxA;
    } else {
        return this->maxB;
    }
}

InteractionHandler::InteractionHandler() {
    this->interactionCounter = new HysteresisCounter(NOINTERACTION_COUNT_MAX, INTERACTION_COUNT_MAX);
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

/*
 * Determines whether an interaction has occurred and if so what type it is
 * Input: interaction data
 * Output: 
 */
bool InteractionHandler::handleInteraction(Interaction *interaction) {
    // Coordinates for a real interaction provided
    bool isInteraction = (interaction != NULL);

    HysteresisCounter::HysteresisValue noInteractionValue = HysteresisCounter::HysteresisValue::A;
    HysteresisCounter::HysteresisValue interactionValue = HysteresisCounter::HysteresisValue::B;

    // A user's click will be sampled multiple times, so recognize whether this is first/last contact
    bool wasOngoingInteraction = (this->interactionCounter->getValue() == interactionValue);
    this->interactionCounter->updateForValue(isInteraction ? interactionValue : noInteractionValue);
    bool isOngoingInteraction = (this->interactionCounter->getValue() == interactionValue);

    // If there was an interaction that just ended, click the mouse up
    if (!isOngoingInteraction && wasOngoingInteraction) {
        std::cout << "InteractionHandler: Interaction STOP" << std::endl;
        this->handleInteractionEndEvent();
    }

    if (!isInteraction) {
        return false;
    }

    // If there is an interaction, move the mouse to the virtual location
    if (isOngoingInteraction) {
        //std::cout << "InteractionHandler: Interaction at x = " << interaction->physicalLocation->x << ", y = " << interaction->physicalLocation->y << ", depth = " << interaction->physicalLocation->z << std::endl;

        this->lastLocation->x = interaction->virtualLocation->x;
        this->lastLocation->y = interaction->virtualLocation->y;
        this->lastTimestamp = interaction->time;

        this->handleInteractionMoveEvent();

        // If this is a new interaction, click the mouse down
        if (!wasOngoingInteraction) {
            std::cout << "InteractionHandler: Interaction START" << std::endl;
            this->firstLocation->x = interaction->virtualLocation->x;
            this->firstLocation->y = interaction->virtualLocation->y;
            this->firstTimestamp = interaction->time;

            this->handleInteractionStartEvent();

            // Output bool only used in calibration
            return true;
        }
    }

    return false;
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

/*
 * Writes comma-separated coordinates and timestamp of a button creation to a file
 */
void InteractionHandler::writeTapLocation(int xpos, int ypos) {
    // get current time
    struct timeval currTimeStruct;
    ::gettimeofday(&currTimeStruct, NULL);
    std::string currTimeStr = std::to_string(currTimeStruct.tv_sec*1000000 + currTimeStruct.tv_usec);
    
    // construct csv string
    std::string xposStr = std::to_string(xpos);
    std::string yposStr = std::to_string(ypos);
    std::string csvStr = xposStr + "," + yposStr +"," + currTimeStr + "\n";
    char *outputBuffer = new char[csvStr.length()+1];
    std::strcpy(outputBuffer, csvStr.c_str());

    std::ofstream outputFile;
    outputFile.open(OUTPUT_FILE, std::fstream::app);
    outputFile << outputBuffer;
    outputFile.close();
    delete []outputBuffer;
}

} /* namespace virtualMonitor */
