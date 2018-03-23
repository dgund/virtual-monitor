/*
    InteractionDetector.h
    Detects interactions with the virtual monitor from sensor data.
*/

#ifndef INTERACTIONDETECTOR_H
#define INTERACTIONDETECTOR_H

#include "KinectReader.h"
#include "Interaction.h"
#include "Viewer.h"

namespace virtualMonitor {

class InteractionDetector {
    public:
        InteractionDetector();
        virtual ~InteractionDetector();

        virtual int start(bool displayViewer=false);
        virtual Interaction *checkForInteraction();
        virtual int stop();

    private:
        KinectReader *reader;
        Viewer *viewer;
        bool displayViewer;
};

} /* namespace virtualMonitor */

#endif /* INTERACTIONDETECTOR_H */
