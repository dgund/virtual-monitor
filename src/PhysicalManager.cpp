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

#define REGRESSION_N 100

#define THRESHOLD_VARIANCE 3000

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
    }

    // Search for the interaction point, starting from the bottom-right of the depth frame and moving right and up
    Interaction *interaction = NULL;
    for (int y = depthFrame->height - 1; 0 <= y; y--) {
        for (int x = 0; x < depthFrame->width; x++) {
            bool isWithinSurfaceBounds = (this->surfaceLeftXForY[y] < depthFrame->width && this->surfaceLeftXForY[y] <= x && x <= this->surfaceRightXForY[y]);
            if (!isWithinSurfaceBounds) {
                if (shouldOutputInteractionPPM) {
                    pixelColors[DEPTH_FRAME_2D_TO_1D(x,y)] = "0 0 255"; // blue
                }
                continue;
            }

            std::string pixelColor = "0 0 0"; // black
            if (!this->isPixelOnSurface(depthFrame, x, y, 2)) {
                pixelColor = "0 255 0"; // green
            } else {
                pixelColor = "255 0 0"; // red
            }

            // If an interaction point has not already been found, check this pixel for an interaction
            if (interaction == NULL) {
                // Anomaly test: If the pixel is an anomoly (not on the surface)
                bool isPixelAnomaly = !this->isPixelOnSurface(depthFrame, x, y, 2);
                if (isPixelAnomaly) {
                    // Edge test: If the pixel is not on the edge of the surface
                    bool isPixelOnSurfaceEdge = this->isPixelOnSurfaceEdge(depthFrame, x, y);
                    if (!isPixelOnSurfaceEdge) {
                        // Size test: If the pixel is significantly large
                        bool isAnomalySignificant = this->isAnomalySizeAtLeast(depthFrame, x, y, 700, 2);
                        if (isAnomalySignificant) {
                            std::cout << "Potential interaction point is (" << x << ", " << y << ")" << std::endl;
                            pixelColor = "0 0 0"; // black

                            // Variance test: If the variance around the pixel is small enough for it to be near the surface
                            float variance = this->findVariance(depthFrame, x, y, 20);
                            std::cout << "Variance: " << variance << "\n";
                            if (variance < THRESHOLD_VARIANCE) {
                                // Pixel is confirmed a point of interaction with the surface
                                interaction = new Interaction();
                                interaction->type = InteractionType::Tap;
                                interaction->time = depthFrame->timestamp;
                                interaction->physicalLocation = new Coord3D();
                                interaction->physicalLocation->x = x;
                                interaction->physicalLocation->y = y;
                                interaction->physicalLocation->z = this->pixelDepth(depthFrame, x, y);;
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
    float depthNext = this->pixelDepth(depthFrame, x, yNext, delta);
    float depthChange = depth - depthNext;

    float surfaceDepth = this->pixelSurfaceRegression(x, y);
    float surfaceDepthNext = this->pixelSurfaceRegression(x, yNext);
    float surfaceDepthChange = surfaceDepth - surfaceDepthNext;

    // Checks if depth is within 200 mm of expected surface depth
    // This is not very agressive to avoid dealing with Kinect innaccuracies
    bool depthSimilarToSurface = std::abs(depth - surfaceDepth) < 200.0;

    // Checks if the change in depth to an adjactent point is within 5 mm of the expected surface change
    bool slopeSimilarToSurface = std::abs(depthChange - surfaceDepthChange) < 5.0;

    return depthSimilarToSurface && slopeSimilarToSurface;
}

bool PhysicalManager::isPixelOnSurfaceEdge(libfreenect2::Frame *depthFrame, int x, int y) {
    return (y+1 > depthFrame->height          || // Pixel is bottom of frame
            this->surfaceLeftXForY[y+1] >= x  || // No surface below pixel
            this->surfaceRightXForY[y+1] <= x ||
            this->surfaceLeftXForY[y] >= x    || // No surface left of pixel
            this->surfaceRightXForY[y] <= x   || // No surface right of pixel
            y-1 < 0                           || // Pixel is top of frame
            this->surfaceLeftXForY[y-1] >= x  || // No surface above pixel
            this->surfaceRightXForY[y-1] <= x);
}

bool PhysicalManager::isAnomalySizeAtLeast(libfreenect2::Frame *depthFrame, int x, int y, int minSize, int delta) {
    int count = 0;

    if (this->isPixelOnSurface(depthFrame, x, y, delta)) {
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
                        bool neighborIsAnomaly = !this->isPixelOnSurface(depthFrame, movingX, movingY, delta);
                        bool neighborIsOnSurfaceEdge = this->isPixelOnSurfaceEdge(depthFrame, movingX, movingY);
                        int neighborIndex = DEPTH_FRAME_2D_TO_1D(movingX,movingY);
                        bool neighborHasNotBeenChecked = (coordsChecked.find(neighborIndex) == coordsChecked.end());
                        if (neighborIsAnomaly && !neighborIsOnSurfaceEdge && neighborHasNotBeenChecked) {
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
                            if (!this->isPixelOnSurface(depthFrame, movingX, movingY, 2)) {
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

int PhysicalManager::findVariance(libfreenect2::Frame *depthFrame, int x, int y, int boxSize) {

	// variance = E[X^2] - E[X]^2, ie (mean of squared data) - (mean of data)^2

	int centerPointDepth = this->pixelDepth(depthFrame, x, y);

	// for each pixel in box around point,
	// compute sum of depths, and sum of depths square
	int lowerBound = (boxSize / 2);
	int upperBound = ((boxSize - 1) / 2);
	int sumDepths = 0;
	int sumSquareDepths = 0;
	int depth = 0;
	for (int movingY = y - lowerBound; movingY <= y + upperBound; movingY++) {
		for (int movingX = x - lowerBound; movingX <= x + upperBound; movingX++) {
			// check that (movingX, movingY) is valid coordinate
			if (movingY >= 0 && movingY < depthFrame->height && movingX >= 0 && movingX < depthFrame->width) {
				// if valid coordinate, use pixel depth for computation
				depth = this->pixelDepth(depthFrame, movingX, movingY);
				sumDepths += depth;
				sumSquareDepths += (depth * depth);
			}
			else {
				// if not valid coordinate, use original point's depth for computation
				// (to minimize variance, because this probably isn't an interaction)
				sumDepths += centerPointDepth;
				sumSquareDepths += (centerPointDepth * centerPointDepth);
			}
		}
	}

	float boxSize_f = (float)(boxSize);
	float meanDepths_f = ((float)sumDepths) / (boxSize_f * boxSize_f);
	float meanSquareDepths_f = ((float)sumSquareDepths) / (boxSize_f * boxSize_f);
	return meanSquareDepths_f - (meanDepths_f * meanDepths_f);    
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
            
            std::string pixelColor = "0 0 0"; // black
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

            float depth = this->pixelDepth(depthFrame, x, y, 2);
            float depthNext = this->pixelDepth(depthFrame, x, yNext, 2);
            float depthChange = depth - depthNext;

            float surfaceDepth = this->pixelSurfaceRegression(x, y);
            float surfaceDepthNext = this->pixelSurfaceRegression(x, yNext);
            float surfaceDepthChange = surfaceDepth - surfaceDepthNext;

            std::string pixelColor = "0 0 0"; // black

            if (DEPTH_MIN < depth && depth < DEPTH_MAX) {
                pixelColor = "0 255 0"; // green
            }

            if (std::abs(depthChange - surfaceDepthChange) < 5.0) {
                pixelColor = "255 0 0"; // red
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
