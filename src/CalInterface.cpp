#include "CalInterface.h"
/*
 * Calibration constructor
 */
Calibrate::Calibrate(const wxString& title, int NumRows, int NumCols, int ButtonSize)
       : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(500,500))
{
    // Create menu bar
    CreateStatusBar(2);
    wxMenuBar *MainMenu = new wxMenuBar();
    SetMenuBar(MainMenu);
   
    // Save inputs
    numRows = NumRows;
    numCols = NumCols;
    buttonSize = ButtonSize; 
   
    // Max screen size
    // -10 because screen is slightly smaller than GetMetric returns
    XMax = wxSystemSettings::GetMetric(wxSYS_SCREEN_X) - 10;
    YMax = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) - 10;
    // Calculations for button positioning
    xInterval = (xMax - buttonSize) / (numCols - 1);
    yInterval = (yMax - buttonSize) / (numRows - 1);
    currIndex = 0;
    
    newButton();
    newButton();
}

/*
 * Add a new yellow calibration button
 */
void Calibrate::newButton()
{
    // Calculate top left corner of calibration button
    int xPos = (currIndex % numCols) * xInterval;
    int yPos = (currIndex / numCols) * yInterval;
    // Center of calibration button
    XPosCenter = xPos + (buttonSize / 2);
    YPosCenter = yPos + (buttonSize / 2);
    
    // Create button image
    wxBitmap bitmap(buttonSize+2, buttonSize+2);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);
    dc.SetBackground(*wxWHITE);
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxYELLOW_BRUSH);
    dc.Clear();
    dc.DrawEllipse(0, 0, buttonSize, buttonSize);
    dc.SelectObject(wxNullBitmap);

    wxBitmapButton *button = new wxBitmapButton(this, wxID_HIGHEST+1, bitmap, wxPoint(xPos,yPos));
    currButton = button;
    this->currButton->Show(true);
}

/*
 * Change calibration button to green
 */
void Calibrate::changeButtonClicked()
{
    wxBitmap bitmap2(buttonSize+2, buttonSize+2);
    wxMemoryDC dc2;
    dc2.SelectObject(bitmap2);
    dc2.SetBackground(*wxWHITE);
    dc2.SetPen(*wxBLACK_PEN);
    dc2.SetBrush(*wxGREEN_BRUSH);
    dc2.Clear();
    dc2.DrawEllipse(0, 0, buttonSize, buttonSize);
    dc2.SelectObject( wxNullBitmap );
    currButton->SetBitmapLabel(bitmap2);
}

/*
 * Change previous button's color to green and add a new yellow button
 */
void Calibrate::NextButton()
{
    changeButtonClicked();
    currIndex++;
    newButton();
}
