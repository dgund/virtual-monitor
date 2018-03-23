/*
    PhysicalManager.cpp
    Processes physical sensor data to detect interactions.
*/

#include "PhysicalManager.h"

#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>

namespace virtualMonitor {

PhysicalManager::PhysicalManager() {

}

PhysicalManager::~PhysicalManager() {

}

Interaction *PhysicalManager::detectInteraction(libfreenect2::Frame* depthFrame) {
    Interaction *interaction = new Interaction();
    return interaction;
}

Interaction *PhysicalManager::detectInteraction(std::string depthFrameFilename) {
    libfreenect2::Frame *depthFrame = NULL;
    readDepthFrameFromFile(depthFrame, depthFrameFilename);
    Interaction *interaction = detectInteraction(depthFrame);
    delete depthFrame;
    return interaction;
}

float PhysicalManager::depthPixel(libfreenect2::Frame* depthFrame, int x, int y) {
    int offset = (y * depthFrame->width) + x;
    int byte_offset = offset * depthFrame->bytes_per_pixel;
    char pixelChar[4] = {depthFrame->data[byte_offset + 0],
                         depthFrame->data[byte_offset + 1],
                         depthFrame->data[byte_offset + 2],
                         depthFrame->data[byte_offset + 3]};
    float pixel;
    memcpy(&pixel, &pixelChar, sizeof(pixel));
    return pixel;
}

int PhysicalManager::readDepthFrameFromFile(libfreenect2::Frame* depthFrame, std::string depthFrameFilename) {
    return 0;
}

int PhysicalManager::writeDepthFrameToFile(libfreenect2::Frame* depthFrame, std::string depthFrameFilename) {
    return 0;
}

int PhysicalManager::writeDepthFrameToPPM(libfreenect2::Frame* depthFrame, std::string ppmFilename) {
    std::ofstream ppmFile(ppmFilename);
    if (!ppmFile.is_open()) {
        return -1;
    }
    int maximumIntensity = 255;
    ppmFile << "P3 " << depthFrame->width << " " << depthFrame->height << " " << maximumIntensity << "\n";

    int y = 0;
    while (y < depthFrame->height) {
        int x = 0;
        while (x < depthFrame->width) {
            float depth = depthPixel(depthFrame, x, y);
            //int depth_image = (int)((depth / 6000) * 255);
            int depth_image = (int)depth % 64;
            ppmFile << depth_image << " " << depth_image << " " << depth_image << " ";
            x++;
        }
        ppmFile << "\n";
        y++;
    }

    ppmFile.close();
    return 0;
}

} /* namespace virtualMonitor */
