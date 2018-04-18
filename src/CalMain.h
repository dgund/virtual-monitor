#include <wx/wx.h>

class MyApp : public wxApp
{
  public:
    virtual bool OnInit();

    private:
        int numRows;
        int numCols;
//        Calibrate *calibrate;
};
