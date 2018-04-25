/*
    InteractionDetector.h
    Detects interactions with the virtual monitor from sensor data.
*/

#ifndef INTERACTIONDETECTOR_H
#define INTERACTIONDETECTOR_H

#include "KinectReader.h"
#include "Interaction.h"
#include "PhysicalManager.h"
<<<<<<< HEAD
=======
#include "Viewer.h"
#include "VirtualManager.h"
>>>>>>> virtual-algorithm

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

    private:
        KinectReader *reader;
        PhysicalManager *physicalManager;
        libfreenect2::Frame *referenceDepthFrame;
        VirtualManager *virtualManager;
};

} /* namespace virtualMonitor */

#endif /* INTERACTIONDETECTOR_H */
