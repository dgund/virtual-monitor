/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Cayla Wanderman-Milne

    VirtualManager.h
    Converts physical coordinates to virtual cooridnates.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VIRTUALMANAGER_H
#define VIRTUALMANAGER_H

#include "Interaction.h"

namespace virtualMonitor {

class VirtualManager {
    private:
        // vars about Kinect's view of screen
        double screenLength_d;
        float A_f;
        float B_f;
        // vars about calibration data
        int calibrationNumRows;
        int calibrationNumCols;
        Coord3D **calibrationCoordsPhysical;
        Coord2D **calibrationCoordsVirtual;
        int *averageYValues; // average y-value of each calibration row
        Coord3D **avgCalCoords; // calibration points with averaged y-values
        // vars about pixel size of screen
        int screenHeightVirtual;
        int screenWidthVirtual;

    public: 
        VirtualManager();
        virtual ~VirtualManager();
        virtual void setCalibrationPoints(int rows, int cols, Coord3D **calibrationCoordsPhysical, Coord2D **calibrationCoordsVirtual);
        virtual void setScreenVirtual(int screenHeightVirtual, int screenWidthVirtual);
        virtual void setVirtualCoord(Interaction *interaction);
    
    private:
        virtual double findArcLength(float A_f, float B_f, int y1, int y2);
        virtual double getXValue(Coord3D *topPoint, Coord3D *bottomPoint, int y);
        virtual void deleteCalibrationVars();
};

} /* namespace virtualMonitor */

#endif /* VIRTUALMANAGER_H */
