/*
    KinectReader.h
    Interfaces with Kinect to read frames for color, depth, and infrared.
*/

#ifndef KINECTREADER_H
#define KINECTREADER_H

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>

namespace virtualMonitor {

struct KinectReaderFrames {
    libfreenect2::Frame *color;
    libfreenect2::Frame *depth;
    libfreenect2::Frame *infrared;
    libfreenect2::Frame *colorDepthRegistered;
    libfreenect2::Frame *colorDepthUndistorted;
    libfreenect2::FrameMap *_frameMap;
};

class KinectReader {

public:
    int timeout;

private:
    libfreenect2::Freenect2 freenect;
    libfreenect2::Freenect2Device *device;
    libfreenect2::SyncMultiFrameListener *listener;
    libfreenect2::Registration *registration;
    bool readColor, readDepthAndInfrared, readColorDepth;

public:
    KinectReader(bool readColor=true, bool readDepthAndInfrared=true, bool readColorDepth=true, int timeout=10000);
    virtual ~KinectReader();

    virtual int start();
    virtual KinectReaderFrames *readFrames();
    virtual int releaseFrames(KinectReaderFrames *frames);
    virtual int stop();
};

} /* namespace virtualMonitor */

#endif /* KINECTREADER_H */
