/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    InteractionDetector.h
    Detects interactions with the virtual monitor from sensor data.

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
        virtual Interaction *detectInteraction(bool isCalibrating=false, bool shouldOutputPPMData=false);
        virtual int stop();
        virtual Interaction *testDetectInteraction(bool shouldOutputPPMData=false);
        virtual int freeInteraction(Interaction *interaction);
        virtual void setScreenVirtual(int screenHeight, int screenWidth);
        virtual void setCalibrationPoints(int rows, int cols, Coord3D **calibrationCoordsPhysical, Coord2D **calibrationCoordsVirtual);

    private:
        KinectReader *reader;
        PhysicalManager *physicalManager;
        libfreenect2::Frame *referenceDepthFrame;
        VirtualManager *virtualManager;
};

} /* namespace virtualMonitor */

#endif /* INTERACTIONDETECTOR_H */
