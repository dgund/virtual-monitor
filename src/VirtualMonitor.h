/*
    VirtualMonitor.h
    Main file for controlling the virtual monitor.
*/

#ifndef VIRTUALMONITOR_H
#define VIRTUALMONITOR_H

#include <atomic>
#include <thread>

#include <wx/wx.h>

#include "CalInterface.h"

#undef VIRTUALMONITOR_TEST_INPUTS
#undef VIRTUALMONITOR_TEST_SNAPSHOT

// Uncomment to use test inputs instead of the live Kinect and output interaction data
//#define VIRTUALMONITOR_TEST_INPUTS

// Uncomment to use a single Kinect snapshot instead of the live Kinect
//#define VIRTUALMONITOR_TEST_SNAPSHOT

enum VirtualMonitorState {
    Paused,
    Detecting,
    Calibrating,
};

class VirtualMonitorApp: public wxApp {
public:
    virtual bool OnInit();
};

class VirtualMonitorFrame: public wxFrame {
// Private variables
private:
    // wxWidgets controls
    wxPanel *panel;
    wxButton *detectButton;
    wxButton *calibrateButton;
    wxStaticText *textLabel;
    // Calibration interface file
    Calibrate *calibrate;
    // Array of physical coordinates of calibration interactions
    Coord3D **calibrationCoords;
    // Whether currently Paused, Detecting, or Calibrating
    VirtualMonitorState state;
    // Threads for interaction managing
    std::thread detectionThread;
    std::thread calibrationThread;
    // Lets detectionThread know that is should stop
    std::atomic<bool> detectionShouldCancel;

// Public methods
public:
    VirtualMonitorFrame();
    virtual ~VirtualMonitorFrame();

// Private methods
private:
    virtual int startDetection();
    virtual int stopDetection();
    virtual void detectionThreadFn();
    virtual void calibrationThreadFn();

    void OnDetect(wxCommandEvent& event);
    void OnCalibrate(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
        
    wxDECLARE_EVENT_TABLE();
};

void detectionThread();
void calibrationThread();

#endif /* VIRTUALMONITOR_H */
