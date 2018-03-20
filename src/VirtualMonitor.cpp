/*
    VirtualMonitor.cpp
    Main file for controlling the virtual monitor.
*/

#include "VirtualMonitor.h"

#include <signal.h>
#include <cstdlib>
#include <iostream>

#include <libfreenect2/libfreenect2.hpp>

#include "KinectReader.h"

#ifdef VIRTUALMONITOR_VIEWER
#include "Viewer.h"
#endif

using namespace virtualMonitor;

// Main runloop
int framesRead(KinectReaderFrames *frames) {
    return 0;
}

// Signal handlers
bool global_shutdown;
void sigintHandler(int s) {
    global_shutdown = true;
}

// Main
int main(int argc, char *argv[]) {
    KinectReader *reader = new KinectReader();
    reader->timeout = VIRTUALMONITOR_DEVICE_TIMEOUT;

    global_shutdown = false;
    signal(SIGINT, sigintHandler);

#ifdef VIRTUALMONITOR_VIEWER
    Viewer viewer;
    viewer.initialize(); 
#endif

    if (reader->start() < 0) {
        std::cout << "Virtual Monitor: Could not start reader." << std::endl;
        global_shutdown = true;
        return -1;
    }

    while (!global_shutdown) {
        KinectReaderFrames *frames = reader->readFrames();
        if (frames == NULL) {
            std::cout << "Virtual Monitor: Could not read frames." << std::endl;
            global_shutdown = true;
            break;
        }

        framesRead(frames);

#ifdef VIRTUALMONITOR_VIEWER
        viewer.addFrame(VIEWER_FRAME_COLOR, frames->color);
        viewer.addFrame(VIEWER_FRAME_INFRARED, frames->infrared);
        viewer.addFrame(VIEWER_FRAME_DEPTH, frames->depth);
        viewer.addFrame(VIEWER_FRAME_REGISTERED, frames->colorDepthRegistered);
        global_shutdown = global_shutdown || viewer.render();
#endif

        reader->releaseFrames(frames);
    }

    reader->stop();

    delete reader;
    return 0;
}
