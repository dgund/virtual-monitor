/*
    KinectReader.cpp
    Interfaces with Kinect to read frames for color, depth, and infrared.
*/

#include "KinectReader.h"

#include <unistd.h>
#include <iostream>

#include <libfreenect2/logger.h>

#define LIBFREENECT_FRAME_NONE 0
#define LIBFREENECT_FRAME_COLOR libfreenect2::Frame::Color
#define LIBFREENECT_FRAME_DEPTH libfreenect2::Frame::Depth
#define LIBFREENECT_FRAME_INFRARED libfreenect2::Frame::Ir

namespace virtualMonitor {

KinectReader::KinectReader(bool readColor, bool readDepthAndInfrared, bool readColorDepth, int timeout) {
    this->readColor = readColor;
    this->readDepthAndInfrared = readDepthAndInfrared;
    this->readColorDepth = readColorDepth;
    this->timeout = timeout;

    // device is allocated in start()
    this->device = NULL;

    // Determine frame types
    int frameTypes = LIBFREENECT_FRAME_NONE;
    if (this->readColor || this->readColorDepth) {
        frameTypes |= LIBFREENECT_FRAME_COLOR;
    }
    if (this->readDepthAndInfrared || this->readColorDepth) {
        frameTypes |= (LIBFREENECT_FRAME_DEPTH | LIBFREENECT_FRAME_INFRARED);
    }

    this->listener = new libfreenect2::SyncMultiFrameListener(frameTypes);

    // registation is allocated in start()
    this->registration = NULL;

    // Disable libfreenect logging for now
    libfreenect2::setGlobalLogger(NULL);
}

KinectReader::~KinectReader() {
    this->stop();
    delete this->listener;
}

int KinectReader::start() {
    if(this->freenect.enumerateDevices() == 0) {
        std::cout << "KinectReader: No device connected." << std::endl;
        return -1;
    }

    std::string deviceSerial = this->freenect.getDefaultDeviceSerialNumber();
    this->pipeline = new libfreenect2::OpenCLPacketPipeline(-1);

    this->device = this->freenect.openDevice(deviceSerial, pipeline);
    if (this->device == NULL) {
        std::cout << "KinectReader: Cannot open device." << std::endl;
        return -1;
    }

    this->device->setColorFrameListener(this->listener);
    this->device->setIrAndDepthFrameListener(this->listener);

    // Start device
    if (!this->device->start()) {
        std::cout << "KinectReader: Cannot start device." << std::endl;
        return -1;
    }

    this->registration = new libfreenect2::Registration(this->device->getIrCameraParams(), this->device->getColorCameraParams());

    return 0;
}

KinectReaderFrames *KinectReader::readFrames() {
    if (this->device == NULL) {
        std::cout << "KinectReader: No device connected." << std::endl;
        return NULL;
    }

    KinectReaderFrames *frames = new KinectReaderFrames;
    frames->_frameMap = new libfreenect2::FrameMap;

    // Read frames
    if (!this->listener->waitForNewFrame(*(frames->_frameMap), this->timeout)) {
        std::cout << "KinectReader: Timeout on new frame." << std::endl;
        return NULL;
    }

    if (this->readColor || this->readColorDepth) {
        frames->color = (*(frames->_frameMap))[LIBFREENECT_FRAME_COLOR];
    }
    if (this->readDepthAndInfrared || this->readColorDepth) {
        frames->depth = (*(frames->_frameMap))[LIBFREENECT_FRAME_DEPTH];
        frames->infrared = (*(frames->_frameMap))[LIBFREENECT_FRAME_INFRARED];
    }
    if (this->readColorDepth) {
        frames->colorDepthRegistered = new libfreenect2::Frame(512, 424, 4);
        frames->colorDepthUndistorted = new libfreenect2::Frame(512, 424, 4);
        registration->apply(frames->color, frames->depth, frames->colorDepthUndistorted, frames->colorDepthRegistered);
    }
    return frames;
}

int KinectReader::releaseFrames(KinectReaderFrames *frames) {
    if (frames->colorDepthRegistered != NULL) {
        delete frames->colorDepthRegistered;
        frames->colorDepthRegistered = NULL;
    }

    if (frames->colorDepthUndistorted != NULL) {
        delete frames->colorDepthUndistorted;
        frames->colorDepthUndistorted = NULL;
    }

    this->listener->release(*(frames->_frameMap));
    frames->color = NULL;
    frames->depth = NULL;
    frames->infrared = NULL;

    delete frames->_frameMap;
    frames->_frameMap = NULL;

    delete frames;
    return 0;
}

int KinectReader::stop() {
    if (this->registration != NULL) {
        delete this->registration;
        this->registration = NULL;
    }
    if (this->device != NULL) {
        this->device->stop();
        this->device->close();
        delete this->device;
        this->device = NULL;
        delete this->pipeline;
        this->pipeline = NULL;
    }

    return 0;
}

} /* namespace virtualMonitor */
