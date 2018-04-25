/*
    InteractionDetector.h
    Detects interactions with the virtual monitor from sensor data.
*/

#ifndef INTERACTIONDETECTOR_H
#define INTERACTIONDETECTOR_H

#include "KinectReader.h"
#include "Interaction.h"
#include "PhysicalManager.h"
#include "VirtualManager.h"

namespace virtualMonitor {

class InteractionDetector {
    public:
        InteractionDetector();
        virtual ~InteractionDetector();

        virtual int start();
        virtual Interaction *detectInteraction(bool shouldOutputPPMData=false);
        virtual int stop();
        virtual Interaction *testDetectInteraction(bool shouldOutputPPMData=false);
        virtual int freeInteraction(Interaction *interaction);
        virtual void setScreenVirtual(int screenHeight, int screenWidth);
        virtual void setCalibrationPoints(int rows, int cols, Coord3D **calibrationCoords);

    private:
        KinectReader *reader;
        PhysicalManager *physicalManager;
        libfreenect2::Frame *referenceDepthFrame;
        VirtualManager *virtualManager;
};

} /* namespace virtualMonitor */

#endif /* INTERACTIONDETECTOR_H */
