#include <wx/wx.h>

class Calibrate: public wxFrame
{
public:
    public:
        Calibrate(const wxString& title, int NumRows, int NumCols, int ButtonSize);
        //void ProcessArgs();
        void NextButton();
        //void OnClick( wxCommandEvent& event );
    
    private:
        int numRows, numCols, buttonSize, xInterval, yInterval, currIndex;
        wxBitmapButton *currButton;
        void newButton();
        void changeButtonClicked();
};
