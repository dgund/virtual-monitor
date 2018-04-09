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

// Signal handlers
bool global_shutdown;
void sigintHandler(int s) {
    global_shutdown = true;
}

// Main
int main(int argc, char *argv[]) {
    global_shutdown = false;
    signal(SIGINT, sigintHandler);

    InteractionDetector *interactionDetector = new InteractionDetector();

    bool outputPPMData = false;
#ifdef VIRTUALMONITOR_OUTPUT_PPM
    outputPPMData = true;
#endif

#ifdef VIRTUALMONITOR_TEST_INPUTS
    // Use test inputs for interaction detection
    interactionDetector->testDetectInteraction(outputPPMData);
#else
    // Use the Kinect sensor for continuous interaction detection
    bool displayViewer = false;
#ifdef VIRTUALMONITOR_OUTPUT_VIEWER
    displayViewer = true;
#endif
    interactionDetector->start(displayViewer);
    while (!global_shutdown) {
        Interaction *interaction = interactionDetector->detectInteraction(outputPPMData);
        if (interaction == NULL) {
            global_shutdown = true;
        }
#ifdef VIRTUALMONITOR_TEST_SNAPSHOT
        // Stop after a single snapshot
        global_shutdown = true;
#endif
    }
    interactionDetector->stop();
#endif

    delete interactionDetector;
    return 0;
}
