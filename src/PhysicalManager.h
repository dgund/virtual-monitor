/*
    PhysicalManager.h
    Processes physical sensor data to detect interactions.
*/

#ifndef PHYSICALMANAGER_H
#define PHYSICALMANAGER_H

#include <string>

#include "Interaction.h"
#include "KinectReader.h"

namespace virtualMonitor {

class PhysicalManager {
    public:
        PhysicalManager();
        virtual ~PhysicalManager();

        virtual Interaction *detectInteraction(libfreenect2::Frame* depthFrame);
        virtual Interaction *detectInteraction(std::string depthFrameFilename);

    private:
        virtual float depthPixel(libfreenect2::Frame* depthFrame, int x, int y);
        virtual libfreenect2::Frame *readDepthFrameFromFile(std::string depthFrameFilename);
        virtual int writeDepthFrameToFile(libfreenect2::Frame* depthFrame, std::string depthFrameFilename);
        virtual int writeDepthFrameToPPM(libfreenect2::Frame* depthFrame, std::string ppmFilename);
};

} /* namespace virtualMonitor */

#endif /* PHYSICALMANAGER_H */
