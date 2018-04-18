#include "CalMain.h"
#include "CalInterface.h"
#include "unistd.h"
#include "thread"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    numRows = 3;
    numCols = 2;
    Calibrate *calibrate = new Calibrate(wxT("Calibrate"), numRows, numCols, 20);
    calibrate->Show(true);
    //std::Thread first(callNextButton);
    return true;
}
/*
void callNextButton() {
    for (int i = 0; i < numRows * numCols; i++) {
        usleep(1000000);
        calibrate->NextButton();
    }
}
*/
