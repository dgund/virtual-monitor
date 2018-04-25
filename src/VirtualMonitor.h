/*
    VirtualMonitor.h
    Main file for controlling the virtual monitor.
*/

#ifndef VIRTUALMONITOR_H
#define VIRTUALMONITOR_H

#include <atomic>
#include <thread>

#include <wx/wx.h>

#include "CalibrationFrame.h"

#undef VIRTUALMONITOR_TEST_INPUTS
#undef VIRTUALMONITOR_TEST_SNAPSHOT

// Uncomment to use test inputs instead of the live Kinect and output interaction data
//#define VIRTUALMONITOR_TEST_INPUTS

// Uncomment to use a single Kinect snapshot instead of the live Kinect
//#define VIRTUALMONITOR_TEST_SNAPSHOT

using namespace virtualMonitor;

enum VirtualMonitorState {
    Paused,
    Detecting,
    Calibrating,
};

class VirtualMonitorApp: public wxApp {
public:
    virtual bool OnInit();
};

DECLARE_EVENT_TYPE(VIRTUALMONITOR_CALIBRATE_THREAD_UPDATE, -1);

class VirtualMonitorCalibrationThread: public wxThread {
protected:
   wxEvtHandler* m_parent;

public:
   explicit VirtualMonitorCalibrationThread(wxEvtHandler* parent) : wxThread(), m_parent(parent) {}

   ExitCode Entry() override;
};

class VirtualMonitorFrame: public wxFrame {
// Private variables
private:
    // wxWidgets controls
    wxPanel *panel;
    wxButton *detectButton;
    wxButton *calibrateButton;
    wxStaticText *textLabel;
    // Calibration frame interface
    CalibrationFrame *calibrationFrame;
    // Array of physical coordinates of calibration interactions
    Coord3D **calibrationPhysicalCoords;
    Coord2D **calibrationVirtualCoords;
    // Whether currently Paused, Detecting, or Calibrating
    VirtualMonitorState state;
    // Threads for interaction managing
    std::thread detectionThread;
    VirtualMonitorCalibrationThread *calibrationThread;
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
    virtual int startCalibration();
    virtual int stopCalibration();
    virtual void detectionThreadFn();
    //virtual void calibrationThreadFn();

    void OnDetect(wxCommandEvent& event);
    void OnCalibrate(wxCommandEvent& event);
    void OnCalibrateThreadUpdate(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
        
    wxDECLARE_EVENT_TABLE();
};

#endif /* VIRTUALMONITOR_H */
