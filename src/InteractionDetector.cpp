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
    this->referenceFrames = NULL;
    this->viewer = new Viewer();
    this->virtualManager = new VirtualManager();
}

InteractionDetector::~InteractionDetector() {
    delete this->reader;
    delete this->physicalManager;
    delete this->viewer;
    delete this->virtualManager;
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

    // Take a snapshot from the Kinect and set that as the reference frame
    this->referenceFrames = this->reader->readFrames();
    if (this->referenceFrames == NULL) {
        std::cout << "Virtual Monitor: Could not read reference frames." << std::endl;
        return -1;
    }
    this->physicalManager->setReferenceFrame(this->referenceFrames->depth);

    return 0;
}

// TODO get rid away 
static int counter = 0;
static Coord3D bottomRight;
static Coord3D topRight;
static Coord3D bottomLeft;
static Coord3D topLeft;

Interaction *InteractionDetector::detectInteraction(bool shouldOutputPPMData) {
    KinectReaderFrames *frames = this->reader->readFrames();
    if (frames == NULL) {
        std::cout << "Virtual Monitor: Could not read frames." << std::endl;
        return NULL;
    }

    std::string interactionPPMFilename = "";
    if (shouldOutputPPMData) {
        interactionPPMFilename = INTERACTION_PPM_FILENAME;
    }

    // Call VirtualManager to check for an interaction (with physical coordinates)
    Interaction *interaction = this->physicalManager->detectInteraction(frames->depth, interactionPPMFilename);

    // TODO get rid of this
    switch (counter) {
        case 0: {
            topLeft = *(interaction->physicalLocation);
            counter++;
            break;
        }
        case 1: {
            topRight = *(interaction->physicalLocation);
            counter++;
            break;
        }
        case 2: {
            bottomLeft = *(interaction->physicalLocation);
            counter++;
            break;
        }
        case 3: {
            bottomRight = *(interaction->physicalLocation);
            this->virtualManager->setScreenPhysical(bottomRight, topRight, bottomLeft, topLeft);
            this->virtualManager->setScreenVirtual(100, 100);
            counter++;
            break;
        }
        default: {
            break;
        }
    }

    if (interaction != NULL) {
        // TODO Call VirtualManager to get virtual coordinates
        // TODO get rid of if statement
        if (counter > 3) {
            this->virtualManager->setVirtualCoord(interaction);
            Coord2D vcoord = *(interaction->virtualLocation);
            std::cout << "VIRTUAL COORDINATE: (" << vcoord.x << ", " << vcoord.y << ")\n";
        }
    }

    if (shouldOutputPPMData) {
        this->physicalManager->writeDepthFrameToPPM(frames->depth, DEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceDepthPPM(frames->depth, SURFACEDEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceSlopePPM(frames->depth, SURFACESLOPE_PPM_FILENAME);
    }

    if (this->displayViewer) {
        this->viewer->addFrame(VIEWER_FRAME_COLOR, frames->color);
        this->viewer->addFrame(VIEWER_FRAME_INFRARED, frames->infrared);
        this->viewer->addFrame(VIEWER_FRAME_DEPTH, frames->depth);
        this->viewer->addFrame(VIEWER_FRAME_REGISTERED, frames->colorDepthRegistered);
        bool shouldStop = this->viewer->render();
        if (shouldStop) {
            // TODO stop detecting interactions because the viewer has been quit
            // This is only a bug for debugging using the viewer
        }
    }

    this->reader->releaseFrames(frames);
    return interaction;
}

int InteractionDetector::stop() {
    this->reader->stop();

    // Free the reference frames set in this->start()
    this->reader->releaseFrames(this->referenceFrames);

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
        this->virtualManager->setVirtualCoord(interaction);
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
