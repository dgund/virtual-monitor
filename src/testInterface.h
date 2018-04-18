#ifndef __TESTINTERFACE_H // Make sure to only declare these classes once
    #define __TESTINTERFACE_H
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <stdlib.h>
#include <time.h>
 
  class MainApp: public wxApp // MainApp is the class for our application
  { // MainApp just acts as a container for the window,
  public: // or frame in MainFrame
    virtual bool OnInit();
  };
 
class MainFrame: public wxFrame // MainFrame is the class for our window,
{ // It contains the window and all objects in it
    public:
        MainFrame( const wxString& title, const wxPoint& pos, const wxSize& size );
        wxPanel *panel;
        wxMenuBar *MainMenu;
        wxBitmapButton unselected;
        wxBitmapButton selected;
        wxBitmapButton *tester;
        void OnClick(wxCommandEvent& event);
        void OnToggleFullScreen(wxCommandEvent& event);
        void OnLeftDown(wxMouseEvent& event);
        DECLARE_EVENT_TABLE()
    private: 
        void SetButton(bool init, int xpos, int ypos);
};
 

  enum
  {
    // declares an id which will be used to call our button
    BUTTON_tester = wxID_HIGHEST + 1,
    Minimal_FullScreen
  };
  
 
  #endif
