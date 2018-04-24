/*
    VirtualManager.cpp
    Converts physical coordinates to virtual cooridnates.
*/

#include "VirtualManager.h"

#include <iostream>
#include <math.h>

namespace virtualMonitor {

/* initalizes private vars of Virtual Manager */
VirtualManager::VirtualManager() {
    this->screenLength_d = 0.0;
    this->A_f = 0.0;
    this->B_f = 0.0;
    this->calibrationNumRows = 0;
    this->calibrationNumCols = 0;
    this->calibrationCoords = NULL;
    this->averageYValues = NULL;
    this->screenHeightVirtual = 0;
    this->screenWidthVirtual = 0;
}

/* frees everything that was created using new() */
VirtualManager::~VirtualManager() {
    // TODO free stuff
}

/* finds the length of a power regression curve between two y-points 
 * inputs: coefficients for power regression (i.e. y = A*z^B),
           y1 and y2 where y1 is bottom-left point and y2 is top-left point 
 * output: length of curve between y1 and y2 */
double VirtualManager::findArcLength(float A_f, float B_f, int y1, int y2) {
    /* y = f(z) = A * z^B   <==>   z = h(y) = (y/A) ^ (1/B)
       length of curve ~= sum from y = y1 to (y2-1) of sqrt(1 + (h(y+1) - h(y))^2)
                        = sum of sqrt(1 + (((y+1)/A) ^ (1/B) - (y/A) ^ (1/B))^2)
    */

    double len_d = 0.0;
    double A_d = (double)A_f;
    double B_d = (double)B_f;
    double invB_d = 1.0 / B_d;

    int yMax = (y1 > y2) ? y1 : y2;
    int yMin = (y1 <= y2) ? y1 : y2;

    for (int y = yMin; y < yMax; y++) {
        double y_d = (double)y;
        double z_d = pow((y_d / A_d), invB_d);
        double zPlus1_d = pow(((y_d+1.0)/A_d), invB_d);

        double zDistSqr_d = pow((zPlus1_d - z_d), 2);
        len_d += sqrt(1.0 + zDistSqr_d);
    }

    // return abs(len_d)
    return (len_d < 0.0) ? (-1.0 * len_d) : len_d;
}

/* sets private vars for calibration points of screen (ie physical coords of screen)
 * inputs: number of rows and number of cols of calibration points,
           array of (pointers to) 3D physical coordinates of calibration points */
void VirtualManager::setCalibrationPoints(int rows, int cols, Coord3D **calibrationCoords) {
    // set private vars
    this->calibrationNumRows = rows;
    this->calibrationNumCols = cols;
    this->calibrationCoords = calibrationCoords;
    this->averageYValues = new int[rows]; //TODO is this how you create a new array?
    // determine average y-value of each row of calibration points
    int currIndex = 0;
    for (int row = 0; row < rows; row++) {
        int sumYValues = 0;
        for (int col = 0; col < cols; col++) {
            sumYValues = calibrationCoords[currIndex]->y;
        }
        this->averageYValues[row] = sumYValues / cols;
        // possible TODO - also compute variance of y-values and print warning if it's high
        //     ie, print warning if the y-values of a row vary a lot
    currIndex++;
    }
}

/* sets private vars for size of screen (in pixels) 
 * inputs: height of screen, width of screen */
void VirtualManager::setScreenVirtual(int screenHeightVirtual, int screenWidthVirtual) {
    this->screenHeightVirtual = screenHeightVirtual;
    this->screenWidthVirtual = screenWidthVirtual;
}

/* takes the physical coordinate of an interaction and updates its 
   virtual coordinate 
 * inputs: interaction whose virtual coordinate should be updated */
void VirtualManager::setVirtualCoord(Interaction *interaction) {
    // make sure VirtualManager private vars have been initialized
    if (this->calibrationCoords == NULL || this->screenHeightVirtual == 0) {
        // TODO print error or check more values?
        std::cout << "setVirtualCoord() called before initial values have been set\n";
        return;
    }

    // if A and B have changed, reset those values and recalculate physical screen height
    if (this->A_f != interaction->surfaceRegressionA ||
        this->B_f != interaction->surfaceRegressionB) {
        
        this->A_f = interaction->surfaceRegressionA;
        this->B_f = interaction->surfaceRegressionB;
        this->screenLength_d = findArcLength(this->A_f, this->B_f, this->averageYValues[0], this->averageYValues[this->calibrationNumRows - 1]);
    }

    /*// set y-coordinate of virtual location
    double interactionHeight_d = findArcLength(this->A_f, this->B_f, this->bottomRight.y, interaction->physicalLocation->y);
    double percentageHeight_d = interactionHeight_d / this->screenLength_d;
    interaction->virtualLocation->y = (int)(((double)screenHeightVirtual) * percentageHeight_d);

    // set x-coordinate of virtual location
    // xLeft_d and xRight_d are the physical x-coordinates of the edge of the screen at height y
    double xLeft_d = xLeftCurve(interaction->physicalLocation->y);
    double xRight_d = xRightCurve(interaction->physicalLocation->y);
    double x_d = (double)(interaction->physicalLocation->x);
    double percentRight = (xLeft_d - x_d) / (xLeft_d - xRight_d);
    interaction->virtualLocation->x = (int)((double)(this->screenWidthVirtual) * percentRight);*/
}

/* returns the physical x-coordinate of the left edge of the screen at height y */
double VirtualManager::xLeftCurve(int y) {
    /*double yBottom_d = (double)(this->bottomLeft.y);
    double yTop_d = (double)(this->topLeft.y);
    double xBottom_d = (double)(this->bottomLeft.x);
    double xTop_d = (double)(this->topLeft.x);
    double y_d = (double)(y);

    // x = (y - y2) * ((x2 - x1)/(y2 - y1)) + x2
    double slopeInverse_d = (xTop_d - xBottom_d) / (yTop_d - yBottom_d);
    return (y_d - yTop_d) * slopeInverse_d + xTop_d;*/
    return 0.0;
}

/* returns the physical x-coordinate of the right edge of the screen at height y */
double VirtualManager::xRightCurve(int y) {
    /*double yBottom_d = (double)(this->bottomRight.y);
    double yTop_d = (double)(this->topRight.y);
    double xBottom_d = (double)(this->bottomRight.x);
    double xTop_d = (double)(this->topRight.x);
    double y_d = (double)(y);

    // x = (y - y2) * ((x2 - x1)/(y2 - y1)) + x2
    double slopeInverse_d = (xTop_d - xBottom_d) / (yTop_d - yBottom_d);
    return (y_d - yTop_d) * slopeInverse_d + xTop_d;*/
    return 0.0;
}

}
