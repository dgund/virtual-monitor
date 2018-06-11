/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    InteractionDetector.cpp
    Detects interactions with the virtual monitor from sensor data.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "InteractionDetector.h"

#include <cstring>
#include <unistd.h>
#include <iostream>

#define DEPTH_PPM_FILENAME "output-depth.ppm"
#define INTERACTION_PPM_FILENAME "output-interaction.ppm"
#define SURFACEDEPTH_PPM_FILENAME "output-surfacedepth.ppm"
#define SURFACESLOPE_PPM_FILENAME "output-surfaceslope.ppm"

namespace virtualMonitor {

/*
 * Constructor for InteractionDetector
 */
InteractionDetector::InteractionDetector() {
    this->reader = new KinectReader();
    this->physicalManager = new PhysicalManager();
    this->referenceDepthFrame = NULL;
    this->virtualManager = new VirtualManager();
}

/*
 * Deconstructor for InteractionDetector
 */
InteractionDetector::~InteractionDetector() {
    delete this->reader;
    delete this->physicalManager;
    delete this->virtualManager;
}

/*
 * Starts reading data from Kinect and saves a reference frame
 */
int InteractionDetector::start() {
    // Check for issues starting Kinect reader
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

/*
 * Gets depth frame from Kinect and determines whether an interaction has occured 
 * Input: isCalibration is whether we are in calibration mode and should not use VirtualManager
 *          calibrationCoords is an array of pointers to physical calibration coordinates
 * Output: pointer to an Interaction (NULL if no interaction occurred)
 */
Interaction *InteractionDetector::detectInteraction(bool isCalibrating, bool shouldOutputPPMData) {
    KinectReaderFrames *frames = this->reader->readFrames();
    // Check for issues reading frames
    if (frames == NULL) {
        std::cout << "VirtualMonitor: Could not read frames." << std::endl;
        return NULL;
    }

    std::string interactionPPMFilename = "";
    if (shouldOutputPPMData) {
        interactionPPMFilename = INTERACTION_PPM_FILENAME;
    }

    // Call PhysicalManager to check for an interaction and update physicalLocation coordiantes
    Interaction *interaction = this->physicalManager->detectInteraction(frames->depth, interactionPPMFilename);

    if (!isCalibrating && interaction != NULL) {
        this->virtualManager->setVirtualCoord(interaction);
        std::cout << "VIRTUAL COORDINATE: (" << interaction->virtualLocation->x << ", " << interaction->virtualLocation->y << ")\n";
    }

    // If option set to output physical depth PPM data, visualize that data
    if (shouldOutputPPMData) {
        this->physicalManager->writeDepthFrameToPPM(frames->depth, DEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceDepthPPM(frames->depth, SURFACEDEPTH_PPM_FILENAME);
        this->physicalManager->writeDepthFrameToSurfaceSlopePPM(frames->depth, SURFACESLOPE_PPM_FILENAME);
    }

    // If current frame is the reference frame, a new reference is needed
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

/*
 * Used for pre-supplied depth frame instead of live stream
 * Inputs: shouldOutputPPMData is whether depth frame view should be saved (as PPM file)
 * Outputs: pointer to an Interaction (NULL if no interaction occurred)
 */
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

    // If option set to output physical depth PPM data, visualize that data
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

void InteractionDetector::setScreenVirtual(int screenHeight, int screenWidth) {
    this->virtualManager->setScreenVirtual(screenHeight, screenWidth);
}

void InteractionDetector::setCalibrationPoints(int rows, int cols, Coord3D **calibrationCoordsPhysical, Coord2D **calibrationCoordsVirtual) {
    this->virtualManager->setCalibrationPoints(rows, cols, calibrationCoordsPhysical, calibrationCoordsVirtual);
}

} /* namespace virtualMonitor */
