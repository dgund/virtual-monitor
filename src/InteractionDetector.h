/*
    InteractionDetector.h
    Detects interactions with the virtual monitor from sensor data.
*/

#ifndef INTERACTIONDETECTOR_H
#define INTERACTIONDETECTOR_H

#include "KinectReader.h"
#include "Interaction.h"
#include "PhysicalManager.h"

namespace virtualMonitor {

class InteractionDetector {
    public:
        InteractionDetector();
        virtual ~InteractionDetector();

        virtual int start();
        virtual Interaction *detectInteraction(bool isCalibrating=false, bool shouldOutputPPMData=false);
        virtual int stop();
        virtual Interaction *testDetectInteraction(bool shouldOutputPPMData=false);
        virtual int freeInteraction(Interaction *interaction);

    private:
        KinectReader *reader;
        PhysicalManager *physicalManager;
        libfreenect2::Frame *referenceDepthFrame;
};

} /* namespace virtualMonitor */

#endif /* INTERACTIONDETECTOR_H */
