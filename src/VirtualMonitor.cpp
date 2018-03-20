/*
    VirtualMonitor.cpp
    Main file for controlling the virtual monitor.
*/

#include <cstdlib>
#include <iostream>
#include <signal.h>

#include <libfreenect2/libfreenect2.hpp>

#include "KinectReader.h"
#include "Viewer.h"

// Timeout for reading a device frame (10 seconds)
#define FRAME_TIMEOUT 10000

using namespace virtualMonitor;

// Signal handlers
bool shutdown;

void sigintHandler(int s) {
    shutdown = true;
}

// Helper functions for Kinect setup
int registerSignalsForReader(KinectReader *reader);
int runViewerForever(KinectReader *reader);

int main(int argc, char *argv[]) {
    KinectReader *reader = new KinectReader();
    reader->timeout = FRAME_TIMEOUT;

    registerSignalsForReader(reader);
    runViewerForever(reader);
    
    delete reader;
    return 0;
}

int registerSignalsForReader(KinectReader *reader) {
    shutdown = false;
    signal(SIGINT, sigintHandler);
    return 0;
}

int runViewerForever(KinectReader *reader) {
    Viewer viewer;
    viewer.initialize(); 

    if (reader->start() < 0) {
        std::cout << "Virtual Monitor: Could not start reader." << std::endl;
        shutdown = true;
        return -1;
    }

    while (!shutdown) {
        KinectReaderFrames *frames = reader->readFrames();
        if (frames == NULL) {
            std::cout << "Virtual Monitor: Could not read frames." << std::endl;
            shutdown = true;
            break;
        }

        viewer.addFrame("RGB", frames->color);
        viewer.addFrame("ir", frames->infrared);
        viewer.addFrame("depth", frames->depth);
        viewer.addFrame("registered", frames->colorDepthRegistered);

        shutdown = shutdown || viewer.render();

        reader->releaseFrames(frames);
    }

    reader->stop();

    return 0;
}
