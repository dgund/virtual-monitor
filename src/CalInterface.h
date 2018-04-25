#include <wx/wx.h>

class Calibrate: public wxFrame
{
public:
    public:
        Calibrate(const wxString& title, int NumRows, int NumCols, int ButtonSize);
        int XMax, YMax;
        int XPosCenter, YPosCenter;
        void NextButton();
    
    private:
        int numRows, numCols, buttonSize, xInterval, yInterval, currIndex;
        wxBitmapButton *currButton;
        void newButton();
        void changeButtonClicked();
};
