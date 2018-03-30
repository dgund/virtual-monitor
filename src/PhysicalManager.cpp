/*
    PhysicalManager.cpp
    Processes physical sensor data to detect interactions.
*/

#include "PhysicalManager.h"

#include <unistd.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

namespace virtualMonitor {

#define DEPTH_FRAME_WIDTH 512
#define DEPTH_FRAME_HEIGHT 424
#define DEPTH_FRAME_BYTES_PER_PIXEL 4

#define DEPTH_MIN 500
#define DEPTH_MAX 9000

#define REGRESSION_N 50

PhysicalManager::PhysicalManager() {

}

PhysicalManager::~PhysicalManager() {

}

Interaction *PhysicalManager::detectInteraction(libfreenect2::Frame* depthFrame) {
    Interaction *interaction = new Interaction();

    this->writeDepthFrameToPPM(depthFrame, "result-original.ppm");

    // Move up in middle until min depth
    int ySurfaceBottom;
    for (ySurfaceBottom = depthFrame->height - 1; ySurfaceBottom > 0; ySurfaceBottom--) {
        float depth = depthPixel(depthFrame, depthFrame->width / 2, ySurfaceBottom);
        if (depth > DEPTH_MIN && depth < DEPTH_MAX) {
            break;
        }
    }
    
    int xRef = depthFrame->height / 2;
    int yRefLow = ySurfaceBottom - 20;

    float yRefs[REGRESSION_N];
    float depthRefs[REGRESSION_N];
    for (int i = 0; i < REGRESSION_N; i++) {
        yRefs[i] = (float)(yRefLow - i);
        depthRefs[i] = this->avgDepthPixel(depthFrame, xRef, yRefs[i], 0);
    }

    float A = 0;
    float B = 0;
    this->powerRegression(yRefs, depthRefs, REGRESSION_N, &A, &B);

    // Run regression

    // Write image
    std::ofstream slopePPM("result-slope.ppm");
    if (!slopePPM.is_open()) {
        std::cout << "PhysicalManager: Could not write depth frame to ppm." << std::endl;
        return NULL;
    }
    int maximumIntensity = 255;
    slopePPM << "P3 " << depthFrame->width << " " << depthFrame->height << " " << maximumIntensity << "\n";

    // Write image
    std::ofstream distancePPM("result-distance.ppm");
    if (!distancePPM.is_open()) {
        std::cout << "PhysicalManager: Could not write depth frame to ppm." << std::endl;
        return NULL;
    }
    distancePPM << "P3 " << depthFrame->width << " " << depthFrame->height << " " << maximumIntensity << "\n";

    int y = 0;
    while (y < depthFrame->height) {
        int x = 0;
        while (x < depthFrame->width) {
///*
            float depth = this->avgDepthPixel(depthFrame, x, y, 2);
            float depthNext = this->avgDepthPixel(depthFrame, x, y-1, 2);
            float depthChange = depthNext - depth;

            float planeDepth = (A) * std::pow(y, B);
            float planeDepthNext = (A) * std::pow(y-1, B);
            float planeDepthChange = planeDepthNext - planeDepth;

            std::string pixelColor = "0 0 0 ";

            if (depth < DEPTH_MIN || depth > DEPTH_MAX) {
                pixelColor = "0 0 0 "; // black
            } else {
                pixelColor = "255 0 0 "; // red
            }

            if (std::abs(depthChange - planeDepthChange) < 5.0) {
                pixelColor = "0 255 0 "; // green
            }
            slopePPM << pixelColor;
//*/
///*
            depth = this->avgDepthPixel(depthFrame, x, y, 0);
            planeDepth = (A) * std::pow(y, B); // power series
            pixelColor = "0 0 0 ";

            if (depth < DEPTH_MIN || depth > DEPTH_MAX) {
                pixelColor = "0 0 0 "; // black
            } else {
                pixelColor = "100 0 0 "; // dark red
            }

            if (std::abs(depth - planeDepth) < 300) {
                pixelColor = "255 0 0 "; // red
            }

            if (std::abs(depth - planeDepth) < 250) {
                pixelColor = "255 155 0 "; // orange
            }

            if (std::abs(depth - planeDepth) < 200) {
                pixelColor = "255 255 0 "; // yellow
            }

            if (std::abs(depth - planeDepth) < 150) {
                pixelColor = "0 255 0 "; // green
            }

            if (std::abs(depth - planeDepth) < 100) {
                pixelColor = "0 0 255 "; // blue
            }

            if (std::abs(depth - planeDepth) < 50) {
                pixelColor = "255 0 255 "; // purple
            }

            if (std::abs(depth - planeDepth) < 25) {
                pixelColor = "150 150 150 "; // gray
            }

            if (std::abs(depth - planeDepth) < 10) {
                pixelColor = "255 255 255 "; // white
            }
//*/
            distancePPM << pixelColor;
            x++;
        }
        slopePPM << "\n";
        distancePPM << "\n";
        y++;
    }

    slopePPM.close();
    distancePPM.close();

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

float PhysicalManager::avgDepthPixel(libfreenect2::Frame* depthFrame, int x, int y, int delta) {
    float depthSum = 0;
    int depthSize = 0;
    for (int pixelY = y - delta; pixelY <= y + delta; pixelY++) {
        if (0 <= pixelY && pixelY < depthFrame->height) {
            for (int pixelX = x - delta; pixelX <= x + delta; pixelX++) {
                if (0 <= pixelX && pixelX < depthFrame->width) {
                    float depth = depthPixel(depthFrame, pixelX, pixelY);
                    depthSum += depth;
                    depthSize++;
                }
            }
        }
    }
    float depthAvg = 0;
    if (depthSize > 0) {
        depthAvg = depthSum / (float)depthSize;
    }
    return depthAvg;
}

void PhysicalManager::powerRegression(float *x, float *y, int n, float *a, float *b) {
    float sumLnX = 0; // sum ln(x)
    float sumLnY = 0; // sum ln(y)
    float sumLnXLnX = 0; // sum ln(x)^2
    float sumLnXLnY = 0; // sum ln(x) * ln(y)
    
    for (int i = 0; i < n; i++) {
        float lnX = std::log(x[i]);
        float lnY = std::log(y[i]);

        sumLnX += lnX;
        sumLnY += lnY;
        sumLnXLnX += (lnX * lnX);
        sumLnXLnY += (lnX * lnY);
    }

    *b = (n * sumLnXLnY - sumLnX * sumLnY) / (n * sumLnXLnX - sumLnX * sumLnX);
    *a = std::exp((sumLnY - (*b) * sumLnX) / n);
}

libfreenect2::Frame *PhysicalManager::readDepthFrameFromFile(std::string depthFrameFilename) {
    std::ifstream depthFile(depthFrameFilename, std::ios::binary | std::ios::ate);
    if (!depthFile.is_open()) {
        std::cout << "PhysicalManager: Could not read depth frame." << std::endl;
        return NULL;
    }

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
        std::cout << "PhysicalManager: Could not write depth frame." << std::endl;
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
        std::cout << "PhysicalManager: Could not write depth frame to ppm." << std::endl;
        return -1;
    }
    int maximumIntensity = 255;
    ppmFile << "P3 " << depthFrame->width << " " << depthFrame->height << " " << maximumIntensity << "\n";

    int y = 0;
    while (y < depthFrame->height) {
        int x = 0;
        while (x < depthFrame->width) {
            float depth = depthPixel(depthFrame, x, y);
            //int depth_image = (int)((depth / 5000) * 255);
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
