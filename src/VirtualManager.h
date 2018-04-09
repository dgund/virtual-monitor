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
        int yBottom;
        int yTop;
        int screenHeightVirtual;
        int screenWidthVirtual;

    public: 
        VirtualManager();
        virtual ~VirtualManager();
    
    private:
        virtual double findArcLength(float A_f, float B_f, int y1, int y2);
        virtual void setScreenPhysical(float A_f, float B_f, int yBottom, int yTop);
        virtual void setScreenVirtual(int screenHeightVirtual, int screenWidthVirtual);
        virtual void setVirtualCoord(Interaction *interaction);
};

} /* namespace virtualMonitor */

#endif /* VIRTUALMANAGER_H */
