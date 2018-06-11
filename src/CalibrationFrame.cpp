/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    CalibrationFrame.cpp
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

#include "CalibrationFrame.h"

namespace virtualMonitor {

#define BUTTON_DIAMETER 20
#define SCREEN_MARGIN (BUTTON_DIAMETER / 2)

/*
 * Calibration constructor
 */
CalibrationFrame::CalibrationFrame(int numRows, int numCols)
       : wxFrame(NULL, wxID_ANY, wxT("Virtual Monitor Calibration"), wxDefaultPosition, wxDefaultSize) {   
    this->numRows = numRows;
    this->numCols = numCols;
    this->currentIndex = 0;

    // Maximize frame to full-screen
    this->Maximize(true);
    //this->ShowFullScreen(true);

    // Initialize buttons
    this->initializeButtons();
    
    // Set first button visible
    this->setButtonVisible(this->buttons[this->currentIndex]);
}

CalibrationFrame::~CalibrationFrame() {
    for (int i = 0; i < (this->numRows * this->numCols); i++) {
        delete this->buttons[i];
    }
    delete[] this->buttons;
}

void CalibrationFrame::initializeButtons() {
    int numButtons = this->numRows * this->numCols;
    this->buttons = new wxBitmapButton* [numButtons];
    Coord2D buttonCoord;
    // Initialize each button to its proper coordinates, but keep hidden
    for (int buttonIndex = 0; buttonIndex < numButtons; buttonIndex++) {
        this->buttonCoordinateForIndex(&buttonCoord, buttonIndex, this->numRows, this->numCols);
        wxBitmap bitmap(BUTTON_DIAMETER+2, BUTTON_DIAMETER+2);
        wxMemoryDC dc;
        dc.SelectObject(bitmap);
        dc.SetBackground(*wxWHITE);
        dc.SetPen(*wxBLACK_PEN);
        dc.SetBrush(*wxRED_BRUSH);
        dc.Clear();
        dc.DrawEllipse(0, 0, BUTTON_DIAMETER, BUTTON_DIAMETER);
        dc.SelectObject(wxNullBitmap);
        this->buttons[buttonIndex] = new wxBitmapButton(this, buttonIndex, bitmap, wxPoint(buttonCoord.x, buttonCoord.y));
        this->buttons[buttonIndex]->Show(false);
    }
}

void CalibrationFrame::frameCoordinateForIndex(Coord2D *virtualCoordinate, int index, int numRows, int numCols) {
    int screenWidth, screenHeight;
    this->GetClientSize(&screenWidth, &screenHeight);
    int screenXMin = SCREEN_MARGIN;
    int screenXMax = screenWidth - 2 * SCREEN_MARGIN;
    int screenYMin = SCREEN_MARGIN;
    int screenYMax = screenHeight - 2 * SCREEN_MARGIN;

    int row = index / numCols;
    int col = index % numCols;

    virtualCoordinate->x = screenXMin + (int)(((float)col / (float)(numCols - 1)) * (screenXMax - screenXMin));
    virtualCoordinate->y = screenYMin + (int)(((float)row / (float)(numRows - 1)) * (screenYMax - screenYMin));
}

void CalibrationFrame::virtualCoordinateForIndex(Coord2D *virtualCoordinate, int index, int numRows, int numCols) {
    this->frameCoordinateForIndex(virtualCoordinate, this->currentIndex, this->numRows, this->numCols);
    this->ClientToScreen(&(virtualCoordinate->x), &(virtualCoordinate->y));
}

void CalibrationFrame::displayNextCalibrationPoint() {
    // Set current button to clicked state
    this->setButtonClicked(this->buttons[this->currentIndex]);
    // Increment current index
    this->currentIndex++;
    // Set new current button visible
    if (currentIndex < this->numRows * this->numCols) {
        this->setButtonVisible(this->buttons[this->currentIndex]);
    }
}

void CalibrationFrame::getCurrentCalibrationPoint(Coord2D *virtualCoordinate) {
    this->virtualCoordinateForIndex(virtualCoordinate, this->currentIndex, this->numRows, this->numCols);
}

void CalibrationFrame::buttonCoordinateForIndex(Coord2D *virtualCoordinate, int index, int numRows, int numCols) {
    this->frameCoordinateForIndex(virtualCoordinate, index, numRows, numCols);
    // The button coordinate is the top-left of the button, so subtract the virtual coordinate (its center) by the radius
    virtualCoordinate->x = virtualCoordinate->x - (BUTTON_DIAMETER / 2);
    virtualCoordinate->y = virtualCoordinate->y - (BUTTON_DIAMETER / 2);
}

void CalibrationFrame::setButtonVisible(wxBitmapButton *button) {
    button->Show(true);
}

void CalibrationFrame::setButtonClicked(wxBitmapButton *button) {
    wxBitmap bitmap(BUTTON_DIAMETER+2, BUTTON_DIAMETER+2);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);
    dc.SetBackground(*wxWHITE);
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxGREEN_BRUSH);
    dc.Clear();
    dc.DrawEllipse(0, 0, BUTTON_DIAMETER, BUTTON_DIAMETER);
    dc.SelectObject(wxNullBitmap);
    button->SetBitmapLabel(bitmap);
}

} /* namespace virtualMonitor */
