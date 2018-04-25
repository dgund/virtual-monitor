/*
    CalibrationFrame.h
    The interface for calibration.
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
