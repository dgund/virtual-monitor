/* 
    VirtualManager.h
    Converts physical cooridnates to virtual coordinates
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
