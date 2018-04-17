/*
    VirtualMonitor.h
    Main file for controlling the virtual monitor.
*/

#ifndef VIRTUALMONITOR_H
#define VIRTUALMONITOR_H

#include <atomic>
#include <thread>

#include <wx/wx.h>

#undef VIRTUALMONITOR_TEST_INPUTS
#undef VIRTUALMONITOR_TEST_SNAPSHOT
#undef VIRTUALMONITOR_OUTPUT_PPM
#undef VIRTUALMONITOR_OUTPUT_VIEWER

// Uncomment to use test inputs instead of the live Kinect
//#define VIRTUALMONITOR_TEST_INPUTS

// Uncomment to use a single Kinect snapshot instead of the live Kinect
//#define VIRTUALMONITOR_TEST_SNAPSHOT

// Uncomment to output depth data in depth, surface-depth, surface-slope, and interaction PPMs
//#define VIRTUALMONITOR_OUTPUT_PPM

// Uncomment to enable the Viewer to display live Kinect data
//#define VIRTUALMONITOR_OUTPUT_VIEWER

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
private:
    wxPanel *panel;
    wxButton *detectButton;
    wxButton *calibrateButton;
    wxStaticText *textLabel;

    VirtualMonitorState state;
    std::thread detectionThread;
    std::atomic<bool> detectionShouldCancel;

public:
    VirtualMonitorFrame();
    virtual ~VirtualMonitorFrame();
private:
    virtual int startDetection();
    virtual int stopDetection();
    virtual void detectionThreadFn();

    void OnDetect(wxCommandEvent& event);
    void OnCalibrate(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
    wxDECLARE_EVENT_TABLE();
};

void detectionThread();

#endif /* VIRTUALMONITOR_H */
