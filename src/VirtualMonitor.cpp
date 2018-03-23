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

    bool displayViewer = true;
    interactionDetector->start(displayViewer);

    while (!global_shutdown) {
        Interaction *interaction = interactionDetector->detectInteraction();
        if (interaction == NULL) {
            global_shutdown = true;
        }
    }

    interactionDetector->stop();

    delete interactionDetector;
    return 0;
}
