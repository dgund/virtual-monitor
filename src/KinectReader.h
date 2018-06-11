/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    KinectReader.h
    Interfaces with Kinect to read frames for color, depth, and infrared.

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

#ifndef KINECTREADER_H
#define KINECTREADER_H

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/packet_pipeline.h>
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
    libfreenect2::PacketPipeline *pipeline;
    libfreenect2::Registration *registration;
    bool readColor, readDepthAndInfrared, readColorDepth;

public:
    KinectReader(bool readColor=true, bool readDepthAndInfrared=true, bool readColorDepth=true, int timeout=10000);
    virtual ~KinectReader();

    virtual int start();
    virtual KinectReaderFrames *readFrames();
    virtual int releaseFrames(KinectReaderFrames *frames);
    virtual int stop();
private:
    virtual int open();
    virtual int close();
};

} /* namespace virtualMonitor */

#endif /* KINECTREADER_H */
