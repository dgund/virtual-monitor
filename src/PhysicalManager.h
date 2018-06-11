/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    PhysicalManager.h
    Processes physical sensor data to detect interactions.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PHYSICALMANAGER_H
#define PHYSICALMANAGER_H

#include <string>
#include <vector>

#include "Interaction.h"
#include "KinectReader.h"

namespace virtualMonitor {

class PhysicalManager {
    private:
        libfreenect2::Frame *referenceFrame;
        float *surfaceRegression;
        float surfaceRegressionEqA;
        float surfaceRegressionEqB;
        int *surfaceLeftXForY;
        int *surfaceRightXForY;

    public:
        PhysicalManager();
        virtual ~PhysicalManager();

        virtual libfreenect2::Frame* getReferenceFrame() { return this->referenceFrame; };
        virtual int setReferenceFrame(libfreenect2::Frame *referenceFrame);

        virtual Interaction *detectInteraction(libfreenect2::Frame *depthFrame, std::string interactionPPMFilename="");
        virtual Interaction *detectInteraction(std::string depthFrameFilename, std::string interactionPPMFilename="");

        virtual libfreenect2::Frame *readDepthFrameFromFile(std::string depthFrameFilename);
        virtual int writeDepthFrameToFile(libfreenect2::Frame *depthFrame, std::string depthFrameFilename);
        virtual int writeDepthFrameToPPM(libfreenect2::Frame *depthFrame, std::string ppmFilename);
        virtual int writeDepthFrameToSurfaceDepthPPM(libfreenect2::Frame *depthFrame, std::string ppmFilename);
        virtual int writeDepthFrameToSurfaceSlopePPM(libfreenect2::Frame *depthFrame, std::string ppmFilename);
        virtual int writeDepthPixelColorsToPPM(std::string pixelColors[], std::string ppmFilename);

    private:
        virtual bool isPixelAnomaly(libfreenect2::Frame *depthFrame, int x, int y, int delta=0);
        virtual bool isPixelSurfaceAnomaly(libfreenect2::Frame *depthFrame, int x, int y, int delta=0);
        virtual bool isPixelSurfaceAnomalyEdge(libfreenect2::Frame *depthFrame, int x, int y, int delta=0);
        virtual bool isAnomalySizeAtLeast(libfreenect2::Frame *depthFrame, int x, int y, int minSize, int delta=0);

        virtual float pixelDepth(libfreenect2::Frame *depthFrame, int x, int y, int delta=0);
        virtual float pixelSurfaceRegression(int x, int y);
        virtual bool isPixelOnSurface(libfreenect2::Frame *depthFrame, int x, int y, int delta=0);
        virtual bool isPixelOnReference(libfreenect2::Frame *depthFrame, int x, int y, int delta=0);
        virtual bool isPixelOnSurfaceEdge(libfreenect2::Frame *depthFrame, int x, int y);

        virtual int updateSurfaceRegressionForReference();
        virtual int updateSurfaceBoundsForReference();

        virtual float depthVariance(libfreenect2::Frame *depthFrame, int x, int y, int boxSideLength);
        virtual int powerRegression(float *x, float *y, int n, float *a, float *b);
};

} /* namespace virtualMonitor */

#endif /* PHYSICALMANAGER_H */
