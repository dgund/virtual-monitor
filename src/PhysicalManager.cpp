/*
    PhysicalManager.cpp
    Processes physical sensor data to detect interactions.
*/

#include "PhysicalManager.h"

#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace virtualMonitor {

#define DEPTH_FRAME_WIDTH 512
#define DEPTH_FRAME_HEIGHT 424
#define DEPTH_FRAME_BYTES_PER_PIXEL 4

PhysicalManager::PhysicalManager() {

}

PhysicalManager::~PhysicalManager() {

}

Interaction *PhysicalManager::detectInteraction(libfreenect2::Frame* depthFrame) {
    Interaction *interaction = new Interaction();
    return interaction;
}

Interaction *PhysicalManager::detectInteraction(std::string depthFrameFilename) {
    libfreenect2::Frame *depthFrame = readDepthFrameFromFile(depthFrameFilename);
    Interaction *interaction = detectInteraction(depthFrame);
    free(depthFrame->data);
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

libfreenect2::Frame *PhysicalManager::readDepthFrameFromFile(std::string depthFrameFilename) {
    std::ifstream depthFile(depthFrameFilename, std::ios::binary | std::ios::ate);
    std::ifstream::pos_type pos = depthFile.tellg();
    int byte_count = pos;

    char *data = (char *)malloc(sizeof(char) * byte_count);
    depthFile.seekg(0, std::ios::beg);
    depthFile.read(data, byte_count);
    depthFile.close();

    libfreenect2::Frame *depthFrame = new libfreenect2::Frame(DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT, DEPTH_FRAME_BYTES_PER_PIXEL, (unsigned char *)data);
    return depthFrame;
}

int PhysicalManager::writeDepthFrameToFile(libfreenect2::Frame* depthFrame, std::string depthFrameFilename) {
    std::ofstream depthFile(depthFrameFilename, std::ios::binary);
    if (!depthFile.is_open()) {
        return -1;
    }

    int byte_count = depthFrame->width * depthFrame->height * depthFrame->bytes_per_pixel;
    depthFile.write((char *)depthFrame->data, byte_count);
    depthFile.close();
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
            int depth_image = (int)depth % 256;
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
