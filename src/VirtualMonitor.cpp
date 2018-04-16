/*
    VirtualMonitor.cpp
    Main file for controlling the virtual monitor.
*/

#include "VirtualMonitor.h"

#include <signal.h>
#include <cstdlib>
#include <iostream>

#include "InteractionDetector.h"

using namespace virtualMonitor;

enum InteractionDetectionSource {
    KinectContinuous,
    KinectSnapshot,
    TestInput
};

// Signal handlers
bool global_shutdown;
void sigintHandler(int s) {
    global_shutdown = true;
}

void runInteractionDetection(InteractionDetectionSource source, bool shouldOutputPPMData, bool shouldDisplayViewer);
void interactionDetected(Interaction *interaction);

void runInteractionDetection(InteractionDetectionSource source, bool shouldOutputPPMData, bool shouldDisplayViewer) {
    InteractionDetector *interactionDetector = new InteractionDetector();
    Interaction *interaction;

    // For TestInput, run a test interaction detection
    if (source == InteractionDetectionSource::TestInput) {
        std::cout << "Virtual Monitor: Checking for test interaction..." << std::endl;
        interaction = interactionDetector->testDetectInteraction(shouldOutputPPMData);
        if (interaction != NULL) {
            interactionDetected(interaction);
            interactionDetector->freeInteraction(interaction);
        }
    }

    // For KinectContinuous or KinectSnapshot, start the detector and detect interactions
    else {
        interactionDetector->start(shouldDisplayViewer);
        while (!global_shutdown) {
            std::cout << "Virtual Monitor: Reading frame and checking for interaction..." << std::endl;
            interaction = interactionDetector->detectInteraction(shouldOutputPPMData);
            if (interaction != NULL) {
                interactionDetected(interaction);
                interactionDetector->freeInteraction(interaction);
            }

            // For KinectSnapshot, stop after one snapshot
            if (source == InteractionDetectionSource::KinectSnapshot) {
                global_shutdown = true;
            }
        }

        interactionDetector->stop();
    }

    delete interactionDetector;
}

void interactionDetected(Interaction *interaction) {
    std::cout << "Virtual Monitor: Interaction detected at x = " << interaction->physicalLocation->x << ", y = " << interaction->physicalLocation->y << ", depth = " << interaction->physicalLocation->z << std::endl;
}

// Main
int main(int argc, char *argv[]) {
    global_shutdown = false;
    signal(SIGINT, sigintHandler);

    // Determine interaction detection source from #defines in VirtualMonitor.h
    InteractionDetectionSource source = InteractionDetectionSource::KinectContinuous;
#ifdef VIRTUALMONITOR_TEST_SNAPSHOT
    source = InteractionDetectionSource::KinectSnapshot;
#elif defined VIRTUALMONITOR_TEST_INPUTS
    source = InteractionDetectionSource::TestInput;
#endif

    // Determine if should output PPM data for the interaction detection from #defines in VirtualMonitor.h
    bool shouldOutputPPMData = false;
#ifdef VIRTUALMONITOR_OUTPUT_PPM
    shouldOutputPPMData = true;
#endif

    // Determine if should display live Kinect viewer from #defines in VirtualMonitor.h
    bool shouldDisplayViewer = false;
#ifdef VIRTUALMONITOR_OUTPUT_VIEWER
    shouldDisplayViewer = true;
#endif

    runInteractionDetection(source, shouldOutputPPMData, shouldDisplayViewer);

    return 0;
}
