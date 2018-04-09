/*
    InteractionDetector.cpp
    Detects interactions with the virtual monitor from sensor data.
*/

#include "InteractionDetector.h"

#include <unistd.h>
#include <iostream>

#define DEPTH_PPM_FILENAME "output-depth.ppm"
#define INTERACTION_PPM_FILENAME "output-interaction.ppm"
#define SURFACEDEPTH_PPM_FILENAME "output-surfacedepth.ppm"
#define SURFACESLOPE_PPM_FILENAME "output-surfaceslope.ppm"

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

Interaction *InteractionDetector::detectInteraction(bool outputPPMData) {
    KinectReaderFrames *frames = this->reader->readFrames();
    if (frames == NULL) {
        std::cout << "Virtual Monitor: Could not read frames." << std::endl;
        return NULL;
    }

    std::string interactionPPMFilename = "";
    if (outputPPMData) {
        interactionPPMFilename = INTERACTION_PPM_FILENAME;
    }
    Interaction *interaction = this->physicalManager->detectInteraction(frames->depth, interactionPPMFilename);

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

    if (outputPPMData) {
        this->physicalManager->writeDepthFrameToPPM(frames->depth, DEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceDepthPPM(frames->depth, SURFACEDEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceSlopePPM(frames->depth, SURFACESLOPE_PPM_FILENAME);
    }

    this->reader->releaseFrames(frames);

    return interaction;
}

int InteractionDetector::stop() {
    this->reader->stop();
    return 0;
}

Interaction *InteractionDetector::testDetectInteraction(bool outputPPMData) {
    std::string interactionPPMFilename = "";
    if (outputPPMData) {
        interactionPPMFilename = INTERACTION_PPM_FILENAME;
    }

    std::string depthFrameFilename = "inputs/interaction2.bin";
    libfreenect2::Frame *depthFrame = this->physicalManager->readDepthFrameFromFile(depthFrameFilename);
    Interaction *interaction = this->physicalManager->detectInteraction(depthFrame, interactionPPMFilename);
    
    if (outputPPMData) {
        this->physicalManager->writeDepthFrameToPPM(depthFrame, DEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceDepthPPM(depthFrame, SURFACEDEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceSlopePPM(depthFrame, SURFACESLOPE_PPM_FILENAME);
    }

    free(depthFrame->data);
    delete depthFrame;
    return interaction;
}

} /* namespace virtualMonitor */
