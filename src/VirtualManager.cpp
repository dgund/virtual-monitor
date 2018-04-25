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
    this->avgCalCoords = NULL;
    this->screenHeightVirtual = 0;
    this->screenWidthVirtual = 0;
}

/* frees everything that was created using new() */
VirtualManager::~VirtualManager() {
    this->deleteCalibrationVars();
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
    this->deleteCalibrationVars();
    // set private vars
    this->calibrationNumRows = rows;
    this->calibrationNumCols = cols;
    this->calibrationCoords = calibrationCoords;
    this->averageYValues = new int[rows]; //TODO is this how you create a new array?
    this->avgCalCoords = new Coord3D*[rows*cols];
    // determine average y-value of each row of calibration points
    for (int row = 0; row < rows; row++) {
        int sumYValues = 0;
        for (int col = 0; col < cols; col++) {
            sumYValues += calibrationCoords[row*cols + col]->y;
        }
        this->averageYValues[row] = sumYValues / cols;
        // possible TODO - also compute variance of y-values and print warning if it's high
        //     ie, print warning if the y-values of a row vary a lot
        for (int col = 0; col < cols; col++) {
            Coord3D *currCalPoint = new Coord3D;
            currCalPoint->x = calibrationCoords[row*cols + col]->x;
            currCalPoint->y = this->averageYValues[row];
            currCalPoint->z = calibrationCoords[row*cols + col]->z;
            this->avgCalCoords[row*cols + col] = currCalPoint;
        }
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

    // copy some long variable names to shorter names to make things easier to read
    int interactionY = interaction->physicalLocation->y;
    int interactionX = interaction->physicalLocation->x;
    int numCols = this->calibrationNumCols;
    int numRows = this->calibrationNumRows;

    /*** determine which calibration cell interaction is in ***/
    // determine which calibration rows the interaction is between
    int calibrationRow; // the first calibration row whose y-value >= interaction's y-value
    for (calibrationRow = 1; calibrationRow < numRows-1; calibrationRow++) {
        if (this->averageYValues[calibrationRow] >= interactionY) {
            break; 
        }
    }
    //std::cout << "calibrationRow: " << calibrationRow << std::endl;
    // determine which calibration cols the interaction is between 
    int calibrationCol; // the first calibration col whose x-value >= interaction's x-value
    for (calibrationCol = numCols-2; calibrationCol >= 1; calibrationCol--) {
        Coord3D *topPoint = this->avgCalCoords[(calibrationRow-1) * numCols + calibrationCol];
        Coord3D *bottomPoint = this->avgCalCoords[calibrationRow * numCols + calibrationCol];
        int colXValue = (int)(getXValue(topPoint, bottomPoint, interactionY));
        if (colXValue >= interactionX) {
            break;
        }
    }

    /*** calculate percentRight ***/
    // calculate percentRight of calibrationCol
    double numCols_d = (double)numCols;
    double percentRightCol_d = ((double)calibrationCol) / (numCols_d-1.0);
    // calculate percentRight of interaction within calibration cell
    Coord3D *topLeftPoint = this->avgCalCoords[(calibrationRow-1) * numCols + calibrationCol];
    Coord3D *bottomLeftPoint = this->avgCalCoords[calibrationRow * numCols + calibrationCol];
    double xLeft_d = getXValue(topLeftPoint, bottomLeftPoint, interactionY);
    Coord3D *topRightPoint = this->avgCalCoords[(calibrationRow-1) * numCols + calibrationCol+1];
    Coord3D *bottomRightPoint = this->avgCalCoords[calibrationRow * numCols + calibrationCol+1];
    double xRight_d = getXValue(topRightPoint, bottomRightPoint, interactionY);
    double x_d = (double)(interactionX);
    double percentRightInteraction_d = (xLeft_d - x_d) / (xLeft_d - xRight_d);
    // calculate percentRight
    double percentRight_d = percentRightCol_d + (percentRightInteraction_d / (numCols_d-1.0));

    /*** calculate percentDown ***/
    // calculate percentDown of calibrationRows
    double calibrationRow_d = (double)calibrationRow;
    double numRows_d = (double)numRows;
    double percentDownRow_d = (calibrationRow_d-1.0) / (numRows_d-1.0);
    // calculate percentDown of interaction within calibration cell
    double yGreater_d = (double)(this->averageYValues[calibrationRow]);
    double yLess_d = (double)(this->averageYValues[calibrationRow-1]);
    double y_d = (double)(interactionY);
    double percentDownInteraction_d = (y_d - yLess_d) / (yGreater_d - yLess_d);
    // calculate percentDown
    double percentDown_d = percentDownRow_d + (percentDownInteraction_d / (numRows_d-1.0));

     /*** set virtual coords ***/
     // TODO cap percentRight/Down between 0 and 1
     interaction->virtualLocation->x = (int)(((double)this->screenWidthVirtual) * percentRight_d);
     interaction->virtualLocation->y = (int)(((double)this->screenHeightVirtual) * percentDown_d);

}

/* returns the x-coordinate of the point with y-coordinate y, described by topPoint and bottomPoint *
 * inputs: topPoint and bottomPoint describe a line,
           y is the y-coordinate of the point for which the x-coordinate will be returned */
double VirtualManager::getXValue(Coord3D *topPoint, Coord3D *bottomPoint, int y) {
    double yBottom_d = (double)(bottomPoint->y);
    double yTop_d = (double)(topPoint->y);
    double xBottom_d = (double)(bottomPoint->x);
    double xTop_d = (double)(topPoint->x);
    double y_d = (double)(y);

    // x = (y - y2) * ((x2 - x1)/(y2 - y1)) + x2
    double slopeInverse_d = (xTop_d - xBottom_d) / (yTop_d - yBottom_d);
    return (y_d - yTop_d) * slopeInverse_d + xTop_d;

}

void VirtualManager::deleteCalibrationVars() {
    if (this->averageYValues != NULL) {
        delete []this->averageYValues;
    }
    if (this->avgCalCoords != NULL) {
        for (int i = 0; i < this->calibrationNumRows * this->calibrationNumCols; i++) {
            delete this->avgCalCoords[i];
        }
        delete []avgCalCoords;
    }
}

} // class VirtualManager
