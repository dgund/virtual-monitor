/*
    InteractionDetector.cpp
    Detects interactions with the virtual monitor from sensor data.
*/

#include "InteractionDetector.h"

#include <unistd.h>
#include <iostream>

namespace virtualMonitor {

InteractionDetector::InteractionDetector() {
    this->reader = new KinectReader();
    this->physicalManager = new PhysicalManager();
    this->viewer = new Viewer();
}

InteractionDetector::~InteractionDetector() {
    delete this->reader;
    delete this->physicalManager;
    delete this->viewer;
}

int InteractionDetector::start(bool displayViewer) {
    this->displayViewer = displayViewer;

    if (this->displayViewer) {
        this->viewer->initialize();
    }

    if (this->reader->start() < 0) {
        std::cout << "Interaction Detector: Could not start reader." << std::endl;
        return -1;
    }
    return 0;
}

Interaction *InteractionDetector::detectInteraction() {
    KinectReaderFrames *frames = this->reader->readFrames();
    if (frames == NULL) {
        std::cout << "Virtual Monitor: Could not read frames." << std::endl;
        return NULL;
    }

    Interaction *interaction = this->physicalManager->detectInteraction(frames->depth);

    if (this->displayViewer) {
        this->viewer->addFrame(VIEWER_FRAME_COLOR, frames->color);
        this->viewer->addFrame(VIEWER_FRAME_INFRARED, frames->infrared);
        this->viewer->addFrame(VIEWER_FRAME_DEPTH, frames->depth);
        this->viewer->addFrame(VIEWER_FRAME_REGISTERED, frames->colorDepthRegistered);
        bool shouldStop = this->viewer->render();
        if (shouldStop) {
            return NULL;
        }
    }

    this->reader->releaseFrames(frames);

    return interaction;
}

int InteractionDetector::stop() {
    this->reader->stop();
    return 0;
}

Interaction *InteractionDetector::testDetectInteraction() {
    Interaction *interaction = this->physicalManager->detectInteraction("inputs/interaction2.bin");
    return interaction;
}

} /* namespace virtualMonitor */
