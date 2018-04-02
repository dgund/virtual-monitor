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
    private:
        libfreenect2::Frame *referenceFrame;
        float *surfaceRegression;
    public:
        PhysicalManager();
        virtual ~PhysicalManager();

        virtual libfreenect2::Frame* getReferenceFrame() { return this->referenceFrame; };
        virtual int setReferenceFrame(libfreenect2::Frame *referenceFrame);

        virtual Interaction *detectInteraction(libfreenect2::Frame *depthFrame);
        virtual Interaction *detectInteraction(std::string depthFrameFilename);

    private:
        virtual float pixelDepth(libfreenect2::Frame *depthFrame, int x, int y, int delta=0);
        virtual float pixelSurfaceRegression(int x, int y);
        virtual bool isPixelOnSurface(libfreenect2::Frame *depthFrame, int x, int y, int delta=0);

        virtual int updateSurfaceRegressionForReference();
        virtual int powerRegression(float *x, float *y, int n, float *a, float *b);

        virtual libfreenect2::Frame *readDepthFrameFromFile(std::string depthFrameFilename);
        virtual int writeDepthFrameToFile(libfreenect2::Frame *depthFrame, std::string depthFrameFilename);
        virtual int writeDepthFrameToPPM(libfreenect2::Frame *depthFrame, std::string ppmFilename);
        virtual int writeDepthFrameToSurfaceDepthPPM(libfreenect2::Frame *depthFrame, std::string ppmFilename);
        virtual int writeDepthFrameToSurfaceSlopePPM(libfreenect2::Frame *depthFrame, std::string ppmFilename);
        virtual int writeDepthFrameToInteractionPPM(libfreenect2::Frame *depthFrame, std::string ppmFilename);
};

} /* namespace virtualMonitor */

#endif /* PHYSICALMANAGER_H */
