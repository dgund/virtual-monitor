/* 
    VirtualManager.h
    Converts physical cooridnates to virtual coordinates
*/

#ifndef VIRTUALMANAGER_H
#define VIRTUALMANAGER__H

#include "Interaction.h"

namespace virtualMonitor {

class VirtualManager {
    private:
        double screenLength_d;
        float A_f;
        float B_f;
        Coord3D bottomRight;
        Coord3D bottomLeft;
        Coord3D topRight;
        Coord3D topLeft;
        int screenHeightVirtual;
        int screenWidthVirtual;

    public: 
        VirtualManager();
        virtual ~VirtualManager();
        virtual void setScreenPhysical(Coord3D bottomRight, Coord3D topRight, Coord3D bottomLeft, Coord3D topLeft);
        virtual void setScreenVirtual(int screenHeightVirtual, int screenWidthVirtual);
        virtual void setVirtualCoord(Interaction *interaction);
    
    private:
        virtual double findArcLength(float A_f, float B_f, int y1, int y2);
        virtual double xLeftCurve(int y);
        virtual double xRightCurve(int y);
};

} /* namespace virtualMonitor */

#endif /* VIRTUALMANAGER_H */
