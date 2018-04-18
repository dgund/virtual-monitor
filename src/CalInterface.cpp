#include "CalInterface.h"
Calibrate::Calibrate(const wxString& title, int NumRows, int NumCols, int ButtonSize)
       : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(500,500))
{
    // create menu bar
    CreateStatusBar(2);
    wxMenuBar *MainMenu = new wxMenuBar();
    SetMenuBar(MainMenu);
   
    // save inputs
    numRows = NumRows;
    numCols = NumCols;
    buttonSize = ButtonSize; 
    
    // calculations for button positioning
    int xMax = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
    int yMax = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
    // -10 because screen is slightly smaller than GetMetric returns
    xInterval = (xMax - buttonSize - 10) / (numCols - 1);
    yInterval = (yMax - buttonSize - 10) / (numRows - 1);
    currIndex = 0;
    /*
    int x = 0;
    int y = 0;
    // add button controls with corners at corner of screen
    for (int i = 0; i < numCols; i++) {
        for (int j = 0; j < numRows; j++) {
            x = i*Xinterval;
            y = j*Yinterval;
            wxBitmapButton *button = new wxBitmapButton(this, wxID_HIGHEST+1, bitmap, wxPoint(x,y));
            button->Bind(wxEVT_BUTTON, &Calibrate::OnClick, this);
        }
    }
    */
    newButton();
    newButton();
}
void Calibrate::newButton()
{
    int xPos = (currIndex % numCols) * xInterval;
    int yPos = (currIndex / numCols) * yInterval;
    // create button image
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
}
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
void Calibrate::OnClick( wxCommandEvent& event )
  {
        wxBitmapButton *pButton = wxDynamicCast(event.GetEventObject(), wxBitmapButton);
        ChangeButtonClicked(pButton);
  }
*/

// change previous button's color to green and add a new yellow button
void Calibrate::NextButton()
{
    changeButtonClicked();
    currIndex++;
    newButton();
}
