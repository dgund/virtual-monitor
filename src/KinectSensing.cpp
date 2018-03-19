/*
    KinectSensing.cpp
    Captures and displays frames for color, depth, and infrared.
*/

#include <iostream>
#include <cstdlib>
#include <signal.h>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>

#include "Viewer.h"

// Timeout for reading a device frame (10 seconds)
#define FRAME_TIMEOUT 10000

// Signal handlers
bool sig_global_shutdown;
bool sig_global_pause;
libfreenect2::Freenect2Device *sig_global_pause_device;

void sigint_handler(int s) {
  sig_global_shutdown = true;
}

void sigusr1_handler(int s) {
    if (sig_global_pause_device == NULL) return;
    if (sig_global_pause) sig_global_pause_device->start();
    else sig_global_pause_device->stop();
    sig_global_pause = !sig_global_pause;
}

// Helper functions for Kinect setup
int connectDevice(libfreenect2::Freenect2 *freenect, libfreenect2::Freenect2Device **device);
int registerSignalsForDevice(libfreenect2::Freenect2Device *device);
int runDevice(libfreenect2::Freenect2Device *device, bool captureColor, bool captureDepthAndIr, bool displayViewer);

int main(int argc, char *argv[]) {
    libfreenect2::Freenect2 freenect;
    libfreenect2::Freenect2Device *device;
    
    if (connectDevice(&freenect, &device) < 0) {
        return -1;
    }

    if(registerSignalsForDevice(device) < 0) {
        return -1;
    }

    if (runDevice(device, true, true, true) < 0) {
        return -1;
    }

    return 0;
}

int connectDevice(libfreenect2::Freenect2 *freenect, libfreenect2::Freenect2Device **device) {
    if(freenect->enumerateDevices() == 0) {
        std::cout << "KinectDepth: No device connected." << std::endl;
        return -1;
    }

    std::string deviceSerial = freenect->getDefaultDeviceSerialNumber();
    *device = freenect->openDevice(deviceSerial);
    if (*device == NULL) {
        std::cout << "KinectDepth: Cannot open device." << std::endl;
        return -1;
    }

    return 0;
}

int registerSignalsForDevice(libfreenect2::Freenect2Device *device) {
    sig_global_shutdown = false;
    sig_global_pause = false;
    sig_global_pause_device = device;
    signal(SIGINT, sigint_handler);
    signal(SIGUSR1, sigusr1_handler);
    return 0;
}

int runDevice(libfreenect2::Freenect2Device *device, bool captureColor, bool captureDepthAndIr, bool displayViewer) {
    // Determine frame types
    int frameTypes = 0;
    if (captureColor) {
        frameTypes |= libfreenect2::Frame::Color;
    }
    if (captureDepthAndIr) {
        frameTypes |= (libfreenect2::Frame::Depth | libfreenect2::Frame::Ir);
    }

    // Initialize frame listener
    libfreenect2::SyncMultiFrameListener listener(frameTypes);
    device->setColorFrameListener(&listener);
    device->setIrAndDepthFrameListener(&listener);

    // Start device
    if (!device->start()) {
        std::cout << "KinectDepth: Cannot start device." << std::endl;
        return -1;
    }

    // Start viewer
    Viewer viewer;
    if (displayViewer) {
        viewer.initialize();
    }

    // Initialize frames
    libfreenect2::FrameMap frames;
    libfreenect2::Registration* registration = new libfreenect2::Registration(device->getIrCameraParams(), device->getColorCameraParams());
    libfreenect2::Frame frame_undistorted(512, 424, 4), frame_registered(512, 424, 4);

    // Loop until shutdown signal
    while (!sig_global_shutdown) {
        // Read frames
        if (!listener.waitForNewFrame(frames, FRAME_TIMEOUT)) {
            std::cout << "KinectDepth: Timeout on new frame." << std::endl;
            return -1;
        }

        // Filter frames
        libfreenect2::Frame *frame_color = frames[libfreenect2::Frame::Color];
        libfreenect2::Frame *frame_depth = frames[libfreenect2::Frame::Depth];
        libfreenect2::Frame *frame_ir = frames[libfreenect2::Frame::Ir];
        if (captureColor && captureDepthAndIr) {
            registration->apply(frame_color, frame_depth, &frame_undistorted, &frame_registered);
        }

        // Display frames
        if (displayViewer) {
            if (captureColor) {
                viewer.addFrame("RGB", frame_color);
            }
            if (captureDepthAndIr) {
                viewer.addFrame("ir", frame_ir);
                viewer.addFrame("depth", frame_depth);
            }
            if (captureColor && captureDepthAndIr) {
                viewer.addFrame("registered", &frame_registered);
            }

            sig_global_shutdown = sig_global_shutdown || viewer.render();
        }

        // Release frames
        listener.release(frames);
    }

    // Clean up
    device->stop();
    device->close();
    delete registration;
    return 0;
}
