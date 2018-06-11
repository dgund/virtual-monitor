/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    CalibrationFrame.h
    The interface for calibration.

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

#include <wx/wx.h>

#include "Location.h"

#ifndef CALIBRATIONFRAME_H
#define CALIBRATIONFRAME_H

namespace virtualMonitor {

class CalibrationFrame: public wxFrame {
private:
    int numRows, numCols;
    int currentIndex;
    wxBitmapButton **buttons;
public:
    CalibrationFrame(int numRows, int numCols);
    virtual ~CalibrationFrame();
    virtual void displayNextCalibrationPoint();
    virtual void getCurrentCalibrationPoint(Coord2D *virtualCoordinate);
private:
    virtual void initializeButtons();
    virtual void frameCoordinateForIndex(Coord2D *virtualCoordinate, int index, int numRows, int numCols);
    virtual void virtualCoordinateForIndex(Coord2D *virtualCoordinate, int index, int numRows, int numCols);
    virtual void buttonCoordinateForIndex(Coord2D *virtualCoordinate, int index, int numRows, int numCols);
    virtual void setButtonVisible(wxBitmapButton *button);
    virtual void setButtonClicked(wxBitmapButton *button);
};

} /* namespace virtualMonitor */

#endif /* CALIBRATIONFRAME_H */
