/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    VirtualMonitor.h
    Main file for controlling the virtual monitor.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VIRTUALMONITOR_H
#define VIRTUALMONITOR_H

#include <atomic>
#include <thread>

#include <wx/wx.h>

#include "CalibrationFrame.h"
#include "CalibrationInteractionHandler.h"
#include "InteractionDetector.h"
#include "MouseInteractionHandler.h"

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
// Public variables
public:
    // Interaction Detector and Handler
    InteractionDetector *detector;
    CalibrationInteractionHandler *calibrationHandler;
    MouseInteractionHandler *mouseHandler;
    // Calibration data
    Coord3D **calibrationPhysicalCoords;
    Coord2D **calibrationVirtualCoords;
    // Calibration frame interface
    CalibrationFrame *calibrationFrame;
// Private variables
private:
    // wxWidgets controls
    wxPanel *panel;
    wxButton *detectButton;
    wxButton *calibrateButton;
    wxStaticText *textLabel;
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
    virtual int readCalibrationDataFromFile(Coord3D **calibrationPhysicalCoords, Coord2D **calibrationVirtualCoords, std::string calibrationDataFilename);
    virtual int writeCalibrationDataToFile(Coord3D **calibrationPhysicalCoords, Coord2D **calibrationVirtualCoords, std::string calibrationDataFilename);

// Private methods
private:
    virtual int startDetection();
    virtual int stopDetection();
    virtual int startCalibration();
    virtual int stopCalibration();
    virtual void detectionThreadFn();

    void OnDetect(wxCommandEvent& event);
    void OnCalibrate(wxCommandEvent& event);
    void OnCalibrateThreadUpdate(wxCommandEvent& event);
    void OnExit(wxCommandEvent& event);
        
    wxDECLARE_EVENT_TABLE();
};

#endif /* VIRTUALMONITOR_H */
