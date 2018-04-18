// to compile: g++ menu.cpp $(wx-config --cxxflags --libs) -o menu
#include <wx/wxprec.h>
    #ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
 
#include "testInterface.h"

#define BUTTONSIZE 20
#define XMAX (wxSystemSettings::GetMetric(wxSYS_SCREEN_X) - BUTTONSIZE - 10)
#define YMAX (wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) - BUTTONSIZE - 10)

int numClicks;
int x;
int y;

IMPLEMENT_APP(MainApp) // Initializes the MainApp class and tells our program
  // to run it
bool MainApp::OnInit()
{
    MainFrame *MainWin = new MainFrame(wxT("Calibrate"), wxPoint(100,100),
    wxSize(800, 800)); // Create an instance of our frame, or window
    MainWin->Show(TRUE); // show the window
    SetTopWindow(MainWin);// and finally, set it as the main window
 
    numClicks = 0;
    srand (time(NULL));
    x = rand() % XMAX;
    y = rand() % YMAX;
    return TRUE;
}
  
BEGIN_EVENT_TABLE (MainFrame, wxFrame)
// Tell the OS to run OnClick when the corresponding button is pressed
EVT_BUTTON (BUTTON_tester, MainFrame::OnClick) // Tell the OS to run ButtonFrame::OnExit when
EVT_MENU (Minimal_FullScreen, MainFrame::OnToggleFullScreen)
EVT_LEFT_DOWN (MainFrame::OnLeftDown)
END_EVENT_TABLE()
 
MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame((wxFrame *) NULL, -1, title, pos, size)
{
    panel = new wxPanel(this, wxID_ANY);
    
    CreateStatusBar(2);
    MainMenu = new wxMenuBar();
    SetMenuBar(MainMenu);
    wxMenu *fileMenu = new wxMenu;
 
    fileMenu->Append(Minimal_FullScreen, _T("Toggle fullscreen\tF11"), _T("Toggle fullscreen display"));
    SetButton(true, x, y);
}

void MainFrame::OnClick( wxCommandEvent& WXUNUSED(event) )
{
    if (numClicks == 2) {
        numClicks = 0;
        x = rand() % XMAX;
        y = rand() % YMAX;
        SetButton(false, x, y);
    }
    else {
        numClicks++;
    }
}
void MainFrame::OnToggleFullScreen(wxCommandEvent& WXUNUSED(event))
{
   ShowFullScreen(!IsFullScreen());
}
        
void MainFrame::OnLeftDown (wxMouseEvent& WXUNUSED(event))
{
}

void MainFrame::SetButton(bool init, int xpos, int ypos) {
    // bitmap when unselected
    wxBitmap bitmap(BUTTONSIZE+2, BUTTONSIZE+2);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);
    dc.SetBackground(*wxWHITE);
    dc.SetPen(*wxBLACK_PEN);
    dc.SetBrush(*wxRED_BRUSH);
    dc.Clear();
    dc.DrawEllipse(0, 0, BUTTONSIZE, BUTTONSIZE);
    dc.SelectObject(wxNullBitmap);
    
    // bitmap when selected
    wxBitmap bitmap2(BUTTONSIZE+2, BUTTONSIZE+2);
    wxMemoryDC dc2;
    dc2.SelectObject(bitmap2);
    dc2.SetBackground(*wxWHITE);
    dc2.SetPen(*wxBLACK_PEN);
    dc2.SetBrush(*wxGREEN_BRUSH);
    dc2.Clear();
    dc2.DrawEllipse(0, 0, BUTTONSIZE, BUTTONSIZE);
    dc2.SelectObject(wxNullBitmap);
    
    if (!init) {
        tester->Destroy();
    }
    tester = new wxBitmapButton(panel, BUTTON_tester, bitmap, wxPoint(x, y));
    tester->SetBitmapSelected(bitmap2);

}
