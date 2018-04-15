/*
    PhysicalManager.cpp
    Processes physical sensor data to detect interactions.
*/

#include "PhysicalManager.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <assert.h>
#include <unistd.h>

namespace virtualMonitor {

#define DEPTH_FRAME_WIDTH 512
#define DEPTH_FRAME_HEIGHT 424
#define DEPTH_FRAME_BYTES_PER_PIXEL 4

#define DEPTH_FRAME_2D_TO_1D(x,y) (y * DEPTH_FRAME_WIDTH + x)

#define DEPTH_MIN 500
#define DEPTH_MAX 9000

#define DEPTH_SMOOTHING_DELTA 2

#define REGRESSION_N 100
#define VARIANCE_BOX_SIDE_LENGTH 20

#define INTERACTION_SURFACE_DEPTH_DIFFERENCE_MAX 200
#define INTERACTION_SURFACE_SLOPE_DIFFERENCE_MAX 5
#define INTERACTION_REFERENCE_DEPTH_DIFFERENCE_MAX 10
#define INTERACTION_REFERENCE_SLOPE_DIFFERENCE_MAX 5

#define INTERACTION_ANOMALY_SIZE_MIN 700
#define INTERACTION_VARIANCE_MAX 3000

#define PIXEL_DEFAULT "0 0 0"
#define PIXEL_SURFACE "255 0 0"
#define PIXEL_ANOMALY "0 255 0"
#define PIXEL_INTERACTION "0 0 255"

PhysicalManager::PhysicalManager() {
    this->referenceFrame = NULL;
    this->surfaceRegression = new float[DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT];
    this->surfaceLeftXForY = new int[DEPTH_FRAME_HEIGHT];
    this->surfaceRightXForY = new int[DEPTH_FRAME_HEIGHT];
}

PhysicalManager::~PhysicalManager() {
    delete this->surfaceRegression;
    delete[] this->surfaceLeftXForY;
    delete[] this->surfaceRightXForY;
    // Expect this->referenceFrame to be freed externally
}

int PhysicalManager::setReferenceFrame(libfreenect2::Frame *referenceFrame) {
    this->referenceFrame = referenceFrame;
    if (this->referenceFrame != NULL) {
        this->updateSurfaceRegressionForReference();
        this->updateSurfaceBoundsForReference();
    }
    return 0;
}

Interaction *PhysicalManager::detectInteraction(libfreenect2::Frame *depthFrame, std::string interactionPPMFilename) {
    assert(depthFrame->width == DEPTH_FRAME_WIDTH);
    assert(depthFrame->height == DEPTH_FRAME_HEIGHT);
    assert(depthFrame->bytes_per_pixel == DEPTH_FRAME_BYTES_PER_PIXEL);

    // If there is no current referenceFrame, set depthFrame as the referenceFrame
    // This will set surfaceReference data if it does not already exist
    if (this->getReferenceFrame() == NULL) {
        this->setReferenceFrame(depthFrame);
    }

    bool shouldOutputInteractionPPM = interactionPPMFilename.length() > 0;
    std::string *pixelColors;
    if (shouldOutputInteractionPPM) {
        pixelColors = new std::string[depthFrame->width * depthFrame->height];
        for (int y = 0; y < depthFrame->height; y++) {
            for (int x = 0; x < depthFrame->width; x++) {
                pixelColors[DEPTH_FRAME_2D_TO_1D(x,y)] = PIXEL_DEFAULT;
            }
        }
    }

    // Search for the interaction point, starting from the bottom-right of the depth frame and moving right and up
    Interaction *interaction = NULL;
    for (int y = depthFrame->height - 1; 0 <= y; y--) {
        int surfaceLeftX = this->surfaceLeftXForY[y];
        int surfaceRightX = this->surfaceRightXForY[y];
        for (int x = surfaceLeftX; x < depthFrame->width && x < surfaceRightX; x++) {

            float pixelDepth = this->pixelDepth(depthFrame, x, y);
            bool isPixelAnomaly = this->isPixelAnomaly(depthFrame, x, y, DEPTH_SMOOTHING_DELTA);

            std::string pixelColor = PIXEL_DEFAULT;
            if (isPixelAnomaly) {
                pixelColor = PIXEL_ANOMALY;
            } else {
                pixelColor = PIXEL_SURFACE;
            }

            // If an interaction point has not already been found, check this pixel for an interaction
            if (interaction == NULL) {
                // Anomaly test: If the pixel depth differs significantly from the surface and reference
                if (isPixelAnomaly) {
                    // Anomaly edge test: If the pixel is on the edge of the anomaly (where the interaction would be)
                    bool isPixelAnomalyEdge = this->isPixelAnomalyEdge(depthFrame, x, y, DEPTH_SMOOTHING_DELTA);
                    if (isPixelAnomalyEdge) {
                        // Size test: If the anomaly is significantly large
                        bool isAnomalySignificant = this->isAnomalySizeAtLeast(depthFrame, x, y, INTERACTION_ANOMALY_SIZE_MIN, DEPTH_SMOOTHING_DELTA);
                        if (isAnomalySignificant) {
                            std::cout << "Potential interaction point is (" << x << ", " << y << ")" << std::endl;
                            pixelColor = PIXEL_INTERACTION; // blue

                            // Variance test: If the variance around the pixel is small enough for it to be near the surface
                            float variance = this->depthVariance(depthFrame, x, y, VARIANCE_BOX_SIDE_LENGTH);
                            std::cout << "Variance: " << variance << "\n";
                            if (variance <= INTERACTION_VARIANCE_MAX) {
                                // Pixel is confirmed a point of interaction with the surface
                                interaction = new Interaction();
                                interaction->type = InteractionType::Tap;
                                interaction->time = depthFrame->timestamp;
                                interaction->physicalLocation = new Coord3D();
                                interaction->physicalLocation->x = x;
                                interaction->physicalLocation->y = y;
                                interaction->physicalLocation->z = pixelDepth;
                                interaction->virtualLocation = new Coord2D();
                                interaction->surfaceRegressionA = this->surfaceRegressionEqA;
                                interaction->surfaceRegressionB = this->surfaceRegressionEqB;

                                // If no need to output the full interaction PPM, return now
                                if (!shouldOutputInteractionPPM) {
                                    return interaction;
                                }
                            }
                        }
                    }
                }
            }
            if (shouldOutputInteractionPPM) {
                pixelColors[DEPTH_FRAME_2D_TO_1D(x,y)] = pixelColor;
            }
        }
    }
    
    if (shouldOutputInteractionPPM) {
        this->writeDepthPixelColorsToPPM(pixelColors, interactionPPMFilename);
        delete[] pixelColors;
    }

    return interaction;
}

Interaction *PhysicalManager::detectInteraction(std::string depthFrameFilename, std::string interactionPPMFilename) {
    libfreenect2::Frame *depthFrame = this->readDepthFrameFromFile(depthFrameFilename);
    Interaction *interaction = this->detectInteraction(depthFrame, interactionPPMFilename);
    free(depthFrame->data);
    delete depthFrame;
    return interaction;
}

bool PhysicalManager::isPixelAnomaly(libfreenect2::Frame *depthFrame, int x, int y, int delta) {
    return (
        // Pixel is not on the surface, and
        !this->isPixelOnSurface(depthFrame, x, y, delta) &&
        // Pixel is not on the edge of the surface, and
        !this->isPixelOnSurfaceEdge(depthFrame, x, y) &&
        // Pixel is not similar to the reference frame (if the depthFrame is not the reference)
        ((depthFrame == this->referenceFrame) || !this->isPixelOnReference(depthFrame, x, y, delta))
    );
}

bool PhysicalManager::isPixelAnomalyEdge(libfreenect2::Frame *depthFrame, int x, int y, int delta) {
    for (int movingY = y - 1; movingY <= y + 1; movingY++) {
        if (0 <= movingY && movingY < depthFrame->height) {
            for (int movingX = x - 1; movingX <= x + 1; movingX++) {
                if (0 <= movingX && movingX < depthFrame->width) {
                    if (!this->isPixelAnomaly(depthFrame, movingX, movingY, delta)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool PhysicalManager::isAnomalySizeAtLeast(libfreenect2::Frame *depthFrame, int x, int y, int minSize, int delta) {
    int count = 0;

    if (!this->isPixelAnomaly(depthFrame, x, y, delta)) {
        return count;
    }

    // Queue of coordinates to check
    std::queue<Coord2D> coordsToCheck;
    coordsToCheck.push((Coord2D){x, y});
    
    // Set of coordinates already checked (stored as int through DEPTH_FRAME_2D_TO_1D())
    std::set<int> coordsChecked;
    coordsChecked.insert(DEPTH_FRAME_2D_TO_1D(x,y));
    
    while(!coordsToCheck.empty()) {
        // Get a coordinate to check
        Coord2D coord = coordsToCheck.front();
        coordsToCheck.pop();
        
        // Increment the count
        count++;

        if (count >= minSize) {
            return true;
        }

        // Check neighboring coordinates next to or below the coordinate
        for (int movingY = coord.y - 1; movingY <= coord.y + 1; movingY++) {
            if (movingY >= 0 && movingY < depthFrame->height) {
                for (int movingX = coord.x - 1; movingX <= coord.x + 1; movingX++) {
                    if (movingX >= 0 && movingX < depthFrame->width) {
                        // If the neighboring point is also a disturbance, add it to the queue to check
                        bool neighborIsAnomaly = this->isPixelAnomaly(depthFrame, movingX, movingY, delta);
                        int neighborIndex = DEPTH_FRAME_2D_TO_1D(movingX,movingY);
                        bool neighborHasNotBeenChecked = (coordsChecked.find(neighborIndex) == coordsChecked.end());
                        if (neighborIsAnomaly && neighborHasNotBeenChecked) {
                            coordsToCheck.push((Coord2D){movingX, movingY});
                            coordsChecked.insert(neighborIndex);
                        }
                    }
                }
            }
        }
    }
    return false;
}

float PhysicalManager::pixelDepth(libfreenect2::Frame *depthFrame, int x, int y, int delta) {
    float depthSum = 0;
    int depthSize = 0;
    for (int movingY = y - delta; movingY <= y + delta; movingY++) {
        if (0 <= movingY && movingY < depthFrame->height) {
            for (int movingX = x - delta; movingX <= x + delta; movingX++) {
                if (0 <= movingX && movingX < depthFrame->width) {
                    int offset = (movingY * depthFrame->width) + movingX;
                    int byte_offset = offset * depthFrame->bytes_per_pixel;
                    unsigned char depthChar[4] = {depthFrame->data[byte_offset + 0],
                                                  depthFrame->data[byte_offset + 1],
                                                  depthFrame->data[byte_offset + 2],
                                                  depthFrame->data[byte_offset + 3]};
                    float depth;
                    std::memcpy(&depth, &depthChar, sizeof(depth));
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

float PhysicalManager::pixelSurfaceRegression(int x, int y) {
    return this->surfaceRegression[DEPTH_FRAME_2D_TO_1D(x,y)];
}

bool PhysicalManager::isPixelOnSurface(libfreenect2::Frame *depthFrame, int x, int y, int delta) {
    int yNext = y - 1;
    if (y == 0) yNext = y + 1;
    
    float depth = this->pixelDepth(depthFrame, x, y, delta);

    // If the depth is outside of the valid min and max, return false
    bool depthValid = DEPTH_MIN <= depth && depth <= DEPTH_MAX;
    if (!depthValid) {
        return false;
    }

    float depthNext = this->pixelDepth(depthFrame, x, yNext, delta);
    float depthChange = depth - depthNext;

    float surfaceDepth = this->pixelSurfaceRegression(x, y);
    float surfaceDepthNext = this->pixelSurfaceRegression(x, yNext);
    float surfaceDepthChange = surfaceDepth - surfaceDepthNext;

    // Checks if depth is within 200 mm of expected surface depth
    // This is not very agressive to avoid dealing with Kinect innaccuracies
    bool depthSimilarToSurface = std::abs(depth - surfaceDepth) < INTERACTION_SURFACE_DEPTH_DIFFERENCE_MAX;

    // Checks if the change in depth to an adjacent point is within 5 mm of the expected surface change
    bool slopeSimilarToSurface = std::abs(depthChange - surfaceDepthChange) < INTERACTION_SURFACE_SLOPE_DIFFERENCE_MAX;

    return depthSimilarToSurface && slopeSimilarToSurface;
}

bool PhysicalManager::isPixelOnReference(libfreenect2::Frame *depthFrame, int x, int y, int delta) {
    int yNext = y - 1;
    if (y == 0) yNext = y + 1;

    float depth = this->pixelDepth(depthFrame, x, y, delta);
    float depthNext = this->pixelDepth(depthFrame, x, yNext, delta);
    float depthChange = depth - depthNext;

    float referenceDepth = this->pixelDepth(this->referenceFrame, x, y, delta);
    float referenceDepthNext = this->pixelDepth(this->referenceFrame, x, yNext, delta);
    float referenceDepthChange = referenceDepth - referenceDepthNext;

    // Checks if depth is within 10 mm of reference depth
    bool depthSimilarToReference = std::abs(depth - referenceDepth) < INTERACTION_REFERENCE_DEPTH_DIFFERENCE_MAX;

    // Checks if the change in depth to an adjacent point is within 5 mm of the reference change
    bool slopeSimilarToReference = std::abs(depthChange - referenceDepthChange) < INTERACTION_REFERENCE_SLOPE_DIFFERENCE_MAX;

    return depthSimilarToReference && slopeSimilarToReference;
}

bool PhysicalManager::isPixelOnSurfaceEdge(libfreenect2::Frame *depthFrame, int x, int y) {
    return (
        // Pixel is top of frame, or
        y - 1 < 0 ||
        // Pixel is bottom of frame, or
        y + 1 > depthFrame->height ||
        // No surface above pixel, or
        this->surfaceLeftXForY[y - 1] >= x ||
        this->surfaceRightXForY[y - 1] <= x ||
        // No surface to left of pixel, or
        this->surfaceLeftXForY[y] >= x ||
        // No surface to right of pixel, or
        this->surfaceRightXForY[y] <= x ||
        // No surface below pixel
        this->surfaceLeftXForY[y + 1] >= x ||
        this->surfaceRightXForY[y + 1] <= x
    );
}

int PhysicalManager::updateSurfaceRegressionForReference() {
    libfreenect2::Frame *depthFrame = this->referenceFrame;

    // x reference is the center of the frame
    int surfaceCenterX = depthFrame->width / 2;

    // y reference is the bottom of the surface
    int surfaceBottomY;
    for (surfaceBottomY = depthFrame->height - 1; surfaceBottomY > 0; surfaceBottomY--) {
        float depth = this->pixelDepth(depthFrame, surfaceCenterX, surfaceBottomY);
        if (DEPTH_MIN < depth && depth < DEPTH_MAX) {
            break;
        }
    }
    // Subtract a few extra pixels just to be sure
    surfaceBottomY -= 20;

    // Capture surface depth data, starting at the y reference and moving up on the x reference for REGRESSION_N pixels
    float yRefs[REGRESSION_N];
    float depthRefs[REGRESSION_N];
    for (int i = 0; i < REGRESSION_N; i++) {
        yRefs[i] = (float)(surfaceBottomY - i);
        depthRefs[i] = this->pixelDepth(depthFrame, surfaceCenterX, yRefs[i]);
    }

    // Compute power regression parameters A and B
    float A = 0;
    float B = 0;
    this->powerRegression(yRefs, depthRefs, REGRESSION_N, &A, &B);
    this->surfaceRegressionEqA = A;
    this->surfaceRegressionEqB = B;

    // Compute surfaceRegression depths
    for (int y = 0; y < depthFrame->height; y++) {
        float expectedSurfaceDepth = (A) * std::pow(y, B);
        for (int x = 0; x < depthFrame->width; x++) {
            this->surfaceRegression[DEPTH_FRAME_2D_TO_1D(x,y)] = expectedSurfaceDepth;
        }
    }

    return 0;
}

int PhysicalManager::updateSurfaceBoundsForReference() {
    libfreenect2::Frame *depthFrame = this->referenceFrame;

    for (int y = depthFrame->height - 1; 0 <= y; y--) {
        // Determine left and right bounds of the surface
        this->surfaceLeftXForY[y] = depthFrame->width;
        this->surfaceRightXForY[y] = -1;
        for (int x = 0; x < depthFrame->width; x++) {
            bool isSurface = true;
            int delta = 1;
            for (int movingY = y - delta; movingY <= y + delta; movingY++) {
                if (0 <= movingY && movingY < depthFrame->height) {
                    for (int movingX = x - delta; movingX <= x + delta; movingX++) {
                        if (0 <= movingX && movingX < depthFrame->width) {
                            if (!this->isPixelOnSurface(depthFrame, movingX, movingY, DEPTH_SMOOTHING_DELTA)) {
                                isSurface = false;
                            }
                        }
                    }
                }
            }

            if (isSurface) {
                if (this->surfaceLeftXForY[y] >= depthFrame->width) {
                    this->surfaceLeftXForY[y] = x;
                }
                this->surfaceRightXForY[y] = x;
            }
        }
    }

    return 0;
}

float PhysicalManager::depthVariance(libfreenect2::Frame *depthFrame, int x, int y, int boxSideLength) {

	// variance = E[X^2] - E[X]^2, ie (mean of squared data) - (mean of data)^2

	int lowerBound = (boxSideLength / 2);
	int upperBound = ((boxSideLength - 1) / 2);
	long sumDepths = 0;
	long sumSquareDepths = 0;
	for (int movingY = y - lowerBound; movingY <= y + upperBound; movingY++) {
        int surfaceLeftX = this->surfaceLeftXForY[movingY];
        int surfaceRightX = this->surfaceRightXForY[movingY];
		for (int movingX = x - lowerBound; movingX <= x + upperBound; movingX++) {
            bool isPixelInFrame = movingY >= 0 && movingY < depthFrame->height && movingX >= 0 && movingX < depthFrame->width;
            bool isPixelInSurfaceBounds = surfaceLeftX <= movingX &&
             movingX <= surfaceRightX;
			if (isPixelInFrame && isPixelInSurfaceBounds) {
				// Sum pixel depths in box
                // Approximate depth float as long to make addition faster
				long depth = this->pixelDepth(depthFrame, movingX, movingY);
				sumDepths += depth;
				sumSquareDepths += (depth * depth);
			}
			else {
				// If the pixel is outside of the frame and/or surface, use depth 0 (aka add nothing to sum) in order to create large variance, as this is likely not an interaction
			}
		}
	}

	float boxSideLength_f = (float)(boxSideLength);
	float meanDepths_f = ((float)sumDepths) / (boxSideLength_f * boxSideLength_f);
	float meanSquareDepths_f = ((float)sumSquareDepths) / (boxSideLength_f * boxSideLength_f);
    float variance = meanSquareDepths_f - (meanDepths_f * meanDepths_f);
	return variance;
}

int PhysicalManager::powerRegression(float *x, float *y, int n, float *a, float *b) {
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
    return 0;
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

int PhysicalManager::writeDepthFrameToFile(libfreenect2::Frame *depthFrame, std::string depthFrameFilename) {
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

int PhysicalManager::writeDepthFrameToPPM(libfreenect2::Frame *depthFrame, std::string ppmFilename) {
    std::string *pixelColors = new std::string[depthFrame->width * depthFrame->height];

    for (int y = 0; y < depthFrame->height; y++) {
        for (int x = 0; x < depthFrame->width; x++) {
            float depth = pixelDepth(depthFrame, x, y);
            //int depth_image = (int)((depth / 5000) * 255);
            int depth_image = (int)depth % 256;
            std::ostringstream pixelColorStream;
            pixelColorStream << depth_image << " " << depth_image << " " << depth_image;
            pixelColors[DEPTH_FRAME_2D_TO_1D(x,y)] = pixelColorStream.str();
        }
    }

    int result = this->writeDepthPixelColorsToPPM(pixelColors, ppmFilename);
    delete[] pixelColors;
    return result;
}

int PhysicalManager::writeDepthFrameToSurfaceDepthPPM(libfreenect2::Frame *depthFrame, std::string ppmFilename) {
    std::string *pixelColors = new std::string[depthFrame->width * depthFrame->height];

    for (int y = 0; y < depthFrame->height; y++) {
        for (int x = 0; x < depthFrame->width; x++) {
            float depth = this->pixelDepth(depthFrame, x, y);
            float surfaceDepth = this->pixelSurfaceRegression(x, y);
            float depthDifference = std::abs(depth - surfaceDepth);
            
            std::string pixelColor = PIXEL_DEFAULT;
            if (DEPTH_MIN < depth && depth < DEPTH_MAX) {
                pixelColor = "100 0 0"; // dark red
            }
            if (depthDifference < 300) {
                pixelColor = "255 0 0"; // red
            }
            if (depthDifference < 250) {
                pixelColor = "255 155 0"; // orange
            }
            if (depthDifference < 200) {
                pixelColor = "255 255 0"; // yellow
            }
            if (depthDifference < 150) {
                pixelColor = "0 255 0"; // green
            }
            if (depthDifference < 100) {
                pixelColor = "0 0 255"; // blue
            }
            if (depthDifference < 50) {
                pixelColor = "255 0 255"; // purple
            }
            if (depthDifference < 25) {
                pixelColor = "150 150 150"; // gray
            }
            if (depthDifference < 10) {
                pixelColor = "255 255 255"; // white
            }
            pixelColors[DEPTH_FRAME_2D_TO_1D(x,y)] = pixelColor;
        }
    }

    int result = this->writeDepthPixelColorsToPPM(pixelColors, ppmFilename);
    delete[] pixelColors;
    return result;
}

int PhysicalManager::writeDepthFrameToSurfaceSlopePPM(libfreenect2::Frame *depthFrame, std::string ppmFilename) {
    std::string *pixelColors = new std::string[depthFrame->width * depthFrame->height];

    for (int y = 0; y < depthFrame->height; y++) {
        for (int x = 0; x < depthFrame->width; x++) {
            int yNext = y - 1;
            if (y == 0) yNext = y + 1;

            float depth = this->pixelDepth(depthFrame, x, y, DEPTH_SMOOTHING_DELTA);
            float depthNext = this->pixelDepth(depthFrame, x, yNext, DEPTH_SMOOTHING_DELTA);
            float depthChange = depth - depthNext;

            float surfaceDepth = this->pixelSurfaceRegression(x, y);
            float surfaceDepthNext = this->pixelSurfaceRegression(x, yNext);
            float surfaceDepthChange = surfaceDepth - surfaceDepthNext;

            std::string pixelColor = PIXEL_DEFAULT;

            if (DEPTH_MIN < depth && depth < DEPTH_MAX) {
                pixelColor = PIXEL_ANOMALY;
            }

            if (std::abs(depthChange - surfaceDepthChange) < INTERACTION_REFERENCE_SLOPE_DIFFERENCE_MAX) {
                pixelColor = PIXEL_SURFACE;
            }
            
            pixelColors[DEPTH_FRAME_2D_TO_1D(x,y)] = pixelColor;
        }
    }

    int result = this->writeDepthPixelColorsToPPM(pixelColors, ppmFilename);
    delete[] pixelColors;
    return result;
}

int PhysicalManager::writeDepthPixelColorsToPPM(std::string pixelColors[], std::string ppmFilename) {
    std::ofstream ppmFile(ppmFilename);
    if (!ppmFile.is_open()) {
        std::cout << "PhysicalManager: Could not write depth frame to ppm." << std::endl;
        return -1;
    }
    int maximumIntensity = 255;
    ppmFile << "P3 " << DEPTH_FRAME_WIDTH << " " << DEPTH_FRAME_HEIGHT << " " << maximumIntensity << "\n";

    for (int y = 0; y < DEPTH_FRAME_HEIGHT; y++) {
        for (int x = 0; x < DEPTH_FRAME_WIDTH; x++) {
            ppmFile << pixelColors[DEPTH_FRAME_2D_TO_1D(x,y)] << " ";
        }
        ppmFile << "\n";
    }

    ppmFile.close();
    return 0;
}

} /* namespace virtualMonitor */
