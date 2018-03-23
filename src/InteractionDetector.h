/*
    InteractionDetector.h
    Detects interactions with the virtual monitor from sensor data.
*/

#ifndef INTERACTIONDETECTOR_H
#define INTERACTIONDETECTOR_H

#include "KinectReader.h"
#include "Interaction.h"
#include "PhysicalManager.h"
#include "Viewer.h"

namespace virtualMonitor {

class InteractionDetector {
    public:
        InteractionDetector();
        virtual ~InteractionDetector();

        virtual int start(bool displayViewer=false);
        virtual Interaction *detectInteraction();
        virtual int stop();

    private:
        KinectReader *reader;
        PhysicalManager *physicalManager;
        Viewer *viewer;
        bool displayViewer;
};

} /* namespace virtualMonitor */

#endif /* INTERACTIONDETECTOR_H */
