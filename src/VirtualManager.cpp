/*
    VirtualManager.cpp
    Converts physical coordinates to virtual cooridnates.
*/

#include "VirtualManager.h"

#include <math.h>

namespace virtualMonitor {

VirtualManager::VirtualManager() {
    this->screenLength_d = 0.0;
    this->A_f = 0.0;
    this->B_f = 0.0;
    this->yBottom = 0;
    this->yTop = 0;
}

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

    for (int y = y1; y < y2; y++) {
        double y_d = (double)y;
        double z_d = pow((y_d / A_d), invB_d);
        double zPlus1_d = pow(((y_d+1.0)/A_d), invB_d);

        double zDistSqr_d = pow((zPlus1_d - z_d), 2);
        len_d += sqrt(1.0 + zDistSqr_d);
    }

    return len_d;
}

/* sets private vars for size and position of screen 
 * inputs: A and B for power regression, 
           bottom y-value and top y-value of screen */
void VirtualManager::setScreenLength(float A_f, float B_f, int yBottom, int yTop) {
    this->screenLength_d = findArcLength(A_f, B_f, yBottom, yTop);
    this->A_f = A_f;
    this->B_f = B_f;
    this->yBottom = yBottom;
    this->yTop = yTop;
}

}
