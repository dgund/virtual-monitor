/*
    Calibration.h
    Defines calibration data structures.
*/

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "Location.h"

namespace virtualMonitor {

class Calibration {
    public:
        Calibration(int rows, int cols);
        ~Calibration();
};

} /* namespace virtualMonitor */

#endif /* CALIBRATION_H */
