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
    this->referenceDepthFrame = NULL;
}

InteractionDetector::~InteractionDetector() {
    delete this->reader;
    delete this->physicalManager;
}

int InteractionDetector::start() {
    if (this->reader->start() < 0) {
        std::cout << "InteractionDetector: Could not start reader." << std::endl;
        return -1;
    }

    // Take a snapshot from the Kinect for the reference frame
    KinectReaderFrames *frames = this->reader->readFrames();
    if (frames == NULL) {
        std::cout << "InteractionDetector: Could not read reference frames." << std::endl;
        return -1;
    }

    // Copy data to a new frame and save as a reference
    size_t byteCount = frames->depth->width * frames->depth->height * frames->depth->bytes_per_pixel;
    char *referenceDepthData = (char *)malloc(sizeof(char) * byteCount);
    std::memcpy(referenceDepthData, frames->depth->data, byteCount);
    this->referenceDepthFrame = new libfreenect2::Frame(frames->depth->width, frames->depth->height, frames->depth->bytes_per_pixel, (unsigned char *)referenceDepthData);
    this->reader->releaseFrames(frames);
    this->physicalManager->setReferenceFrame(this->referenceDepthFrame);

    return 0;
}

Interaction *InteractionDetector::detectInteraction(bool shouldOutputPPMData) {
    KinectReaderFrames *frames = this->reader->readFrames();
    if (frames == NULL) {
        std::cout << "VirtualMonitor: Could not read frames." << std::endl;
        return NULL;
    }

    std::string interactionPPMFilename = "";
    if (shouldOutputPPMData) {
        interactionPPMFilename = INTERACTION_PPM_FILENAME;
    }

    // Call VirtualManager to check for an interaction (with physical coordinates)
    Interaction *interaction = this->physicalManager->detectInteraction(frames->depth, interactionPPMFilename);

    if (interaction != NULL) {
        // TODO Call VirtualManager to get virtual coordinates
    }

    if (shouldOutputPPMData) {
        this->physicalManager->writeDepthFrameToPPM(frames->depth, DEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceDepthPPM(frames->depth, SURFACEDEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceSlopePPM(frames->depth, SURFACESLOPE_PPM_FILENAME);
    }

    if (frames->depth == this->physicalManager->getReferenceFrame()) {
        this->physicalManager->setReferenceFrame(NULL);
    }
    this->reader->releaseFrames(frames);
    return interaction;
}

int InteractionDetector::stop() {
    this->reader->stop();

    // Free the reference frames set in this->start()
    delete[] this->referenceDepthFrame->data;
    delete this->referenceDepthFrame;
    this->referenceDepthFrame = NULL;

    return 0;
}

Interaction *InteractionDetector::testDetectInteraction(bool shouldOutputPPMData) {
    std::string interactionPPMFilename = "";
    if (shouldOutputPPMData) {
        interactionPPMFilename = INTERACTION_PPM_FILENAME;
    }

    std::string referenceFrameFilename = "inputs/surface.bin";
    libfreenect2::Frame *referenceFrame = this->physicalManager->readDepthFrameFromFile(referenceFrameFilename);
    std::cout << "InteractionDetector: Setting test reference frame..." << std::endl;
    this->physicalManager->setReferenceFrame(referenceFrame);

    std::string depthFrameFilename = "inputs/nointeraction1.bin";
    libfreenect2::Frame *depthFrame = this->physicalManager->readDepthFrameFromFile(depthFrameFilename);
    std::cout << "InteractionDetector: Detecting test interaction..." << std::endl;
    Interaction *interaction = this->physicalManager->detectInteraction(depthFrame, interactionPPMFilename);
    
    if (interaction != NULL) {
        // TODO Call VirtualManager to get virtual coordinates
    }

    if (shouldOutputPPMData) {
        this->physicalManager->writeDepthFrameToPPM(depthFrame, DEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceDepthPPM(depthFrame, SURFACEDEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceSlopePPM(depthFrame, SURFACESLOPE_PPM_FILENAME);
    }

    free(depthFrame->data);
    delete depthFrame;

    free(referenceFrame->data);
    delete referenceFrame;

    return interaction;
}

int InteractionDetector::freeInteraction(Interaction *interaction) {
    if (interaction != NULL) {
        if (interaction->physicalLocation != NULL) {
            delete interaction->physicalLocation;
            interaction->physicalLocation = NULL;
        }

        if (interaction->virtualLocation != NULL) {
            delete interaction->virtualLocation;
            interaction->virtualLocation = NULL;
        }

        delete interaction;
        interaction = NULL;
    }

    return 0;
}

} /* namespace virtualMonitor */
