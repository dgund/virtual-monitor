/* 
    VirtualManager.h
    Converts physical cooridnates to virtual coordinates
*/

#ifndef VIRTUALMANAGER_H
#define VIRTUALMANAGER__H

namespace virtualMonitor {

class VirtualManager {
    private:
        double screenLength_d;
        float A_f;
        float B_f;
        int yBottom;
        int yTop;

    public: 
        VirtualManager();
        virtual ~VirtualManager();
    
    private:
        virtual double findArcLength(float A_f, float B_f, int y1, int y2);
        virtual void setScreenLength(float A_f, float B_f, int yBottom, int yTop);
};

} /* namespace virtualMonitor */

#endif /* VIRTUALMANAGER_H */
