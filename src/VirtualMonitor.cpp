/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    VirtualMonitor.cpp
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

#include "VirtualMonitor.h"

#include <fstream>
#include <iostream>

#include "CalibrationInteractionHandler.h"
#include "InteractionDetector.h"
#include "MouseInteractionHandler.h"

#define LABEL_START_DETECTION "Start Detection"
#define LABEL_STOP_DETECTION "Stop Detection"
#define LABEL_CALIBRATE "Calibrate"

// How many calibration rows and cols
#define CALIBRATION_ROWS 3
#define CALIBRATION_COLS 3

#define CALIBRATION_DATA_FILENAME "calibration.vmcal"

using namespace virtualMonitor;

/*** VirtualMonitorApp ***/
wxIMPLEMENT_APP(VirtualMonitorApp);

// Create and show visual frame
bool VirtualMonitorApp::OnInit() {
    VirtualMonitorFrame *frame = new VirtualMonitorFrame();
    frame->Show(true);
    return true;
}

/*** VirtualMonitorFrame ***/
enum {
    ID_DETECT_BTN = 1,
    ID_CALIBRATE_BTN = 2
};

DEFINE_EVENT_TYPE(VIRTUALMONITOR_CALIBRATE_THREAD_UPDATE);

/*
 * Event table for handling user interacting with controls
 */
wxBEGIN_EVENT_TABLE(VirtualMonitorFrame, wxFrame)
    EVT_MENU(ID_DETECT_BTN, VirtualMonitorFrame::OnDetect)
    EVT_MENU(ID_CALIBRATE_BTN, VirtualMonitorFrame::OnCalibrate)
    EVT_COMMAND(wxID_ANY, VIRTUALMONITOR_CALIBRATE_THREAD_UPDATE, VirtualMonitorFrame::OnCalibrateThreadUpdate)
wxEND_EVENT_TABLE()

/*
 * Visual frame constructor
 */
VirtualMonitorFrame::VirtualMonitorFrame() : wxFrame(NULL, wxID_ANY, wxT("Virtual Monitor"), wxDefaultPosition, wxSize(180, 120)) {
    this->state = VirtualMonitorState::Paused;

    // Create panel within frame
    this->panel = new wxPanel(this, wxID_ANY);
    this->panel->Show(true);

    // Add controls to panel
    this->detectButton = new wxButton(this->panel, ID_DETECT_BTN, wxT(LABEL_START_DETECTION), wxPoint(10, 10));
    Connect(ID_DETECT_BTN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(VirtualMonitorFrame::OnDetect));
    this->detectButton->Show(true);

    this->calibrateButton = new wxButton(this->panel, ID_CALIBRATE_BTN, wxT(LABEL_CALIBRATE), wxPoint(10, 40));
    Connect(ID_CALIBRATE_BTN, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(VirtualMonitorFrame::OnCalibrate));
    this->calibrateButton->Show(true);

    // Show text that might be used to help user
    // TODO currently filler text
    this->textLabel = new wxStaticText(this->panel, 0, wxT("Text"), wxPoint(10, 70));
    this->textLabel->Show(true);

    panel->Fit();

    // Initialize arrays of physical and virtual calibration data
    this->calibrationPhysicalCoords = new Coord3D* [CALIBRATION_ROWS * CALIBRATION_COLS];
    for (int i = 0; i < CALIBRATION_ROWS * CALIBRATION_COLS; i++) {
        this->calibrationPhysicalCoords[i] = new Coord3D();
    }
    this->calibrationVirtualCoords = new Coord2D* [CALIBRATION_ROWS * CALIBRATION_COLS];
    for (int i = 0; i < CALIBRATION_ROWS * CALIBRATION_COLS; i++) {
        this->calibrationVirtualCoords[i] = new Coord2D();
    }

    this->calibrationFrame = NULL;

    this->detector = new InteractionDetector();
    this->calibrationHandler = new CalibrationInteractionHandler();
    this->mouseHandler = new MouseInteractionHandler();
}

/*
 * Visual frame deconstructor
 */
VirtualMonitorFrame::~VirtualMonitorFrame() {
    // Stop detection thread if running
    if (this->state == VirtualMonitorState::Detecting) {
        this->stopDetection();
    }

    delete this->textLabel;
    delete this->calibrateButton;
    delete this->detectButton;
    delete this->panel;
    for (int i = 0; i < CALIBRATION_ROWS * CALIBRATION_COLS; i++) {
        delete this->calibrationPhysicalCoords[i];
    }
    delete[] this->calibrationPhysicalCoords;
    for (int i = 0; i < CALIBRATION_ROWS * CALIBRATION_COLS; i++) {
        delete this->calibrationVirtualCoords[i];
    }
    delete[] this->calibrationVirtualCoords;

    delete this->detector;
    delete this->calibrationHandler;
    delete this->mouseHandler;
}

/*
 * Event handler for when user presses "Start Detection" or "Stop Detection" button
 */
void VirtualMonitorFrame::OnDetect(wxCommandEvent& event) {
    switch (this->state) {
    // If already calibrating, do nothing
    case VirtualMonitorState::Calibrating:
        return;
    // If currently detecting, pause
    case VirtualMonitorState::Detecting:
        // Stop detection
        this->stopDetection();
        this->state = VirtualMonitorState::Paused;
        break;
    // If currently paused, start detection
    case VirtualMonitorState::Paused:
        // Start detecting
        this->startDetection();
        this->state = VirtualMonitorState::Detecting;
        break;
    }

    // Toggle label on detection button
    this->detectButton->SetLabel(this->state == VirtualMonitorState::Detecting ?
            LABEL_STOP_DETECTION : LABEL_START_DETECTION);
}

/*
 * Event handler for when user presses "Calibrate" button
 */
void VirtualMonitorFrame::OnCalibrate(wxCommandEvent& event) {
    switch (this->state) {
    // If already calibrating, do nothing
    case VirtualMonitorState::Calibrating:
        return;
    // If currently detecting, pause and start calibration
    case VirtualMonitorState::Detecting:
        // Stop detection
        this->stopDetection();
        this->state = VirtualMonitorState::Paused;
        // continue
    // If currently paused, start calibration
    case VirtualMonitorState::Paused:
        // Start calibration
        this->state = VirtualMonitorState::Calibrating;
        // Start calibrating
        this->startCalibration();
        break;
    }
}

void VirtualMonitorFrame::OnCalibrateThreadUpdate(wxCommandEvent& event) {
    std::cout << "Calibration update..." << std::endl;
    int calibrationIndex = event.GetInt();
    if (calibrationIndex < (CALIBRATION_ROWS * CALIBRATION_COLS) - 1) {
        // Show next calibration point
        this->calibrationFrame->displayNextCalibrationPoint();
    } else {
        this->stopCalibration();
    }
    std::cout << "Calibration update done..." << std::endl;
}

/*
 * Event handler for when user closes window
 */
void VirtualMonitorFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

/*** Detection ***/

/*
 * Starts detection, spawns detectionThread
 *  to continuously read Kinect data and look for interactions
 */
int VirtualMonitorFrame::startDetection() {
    // Read calibration from file
    this->readCalibrationDataFromFile(this->calibrationPhysicalCoords, this->calibrationVirtualCoords, CALIBRATION_DATA_FILENAME);
    // Reset cancellation token
    this->detectionShouldCancel = false;
    // Start interaction detection/handling on new thread
    this->detectionThread = std::thread(&VirtualMonitorFrame::detectionThreadFn, this);
    return 0;
}

/*
 * Stops detection, joins detectionThread
 */
int VirtualMonitorFrame::stopDetection() {
    this->detectionShouldCancel = true;
    this->detectionThread.join();
    return 0;
}

/*  
 * Continuously reads Kinect data and looks for interactions
 */
void VirtualMonitorFrame::detectionThreadFn() {
#ifdef VIRTUALMONITOR_TEST_INPUTS
    // Detect interaction with isCalibrating = false, outputPPMData = true
    Interaction *interaction = this->detector->testDetectInteraction(false, true);
    // Handle interaction
    this->mouseHandler->handleInteraction(interaction);
    if (interaction != NULL) {
        // Free interaction
        this->detector->freeInteraction(interaction);
    }
#else
    // Pass in calibration data to be used by virtualManager
    this->detector->setCalibrationPoints(CALIBRATION_ROWS, CALIBRATION_COLS, this->calibrationPhysicalCoords, this->calibrationVirtualCoords);
    this->detector->setScreenVirtual(wxSystemSettings::GetMetric(wxSYS_SCREEN_Y), wxSystemSettings::GetMetric(wxSYS_SCREEN_X));

    // Check for errors in starting detector
    if (this->detector->start() < 0) {
        return;
    }

    std::cout << "Starting detection..." << std::endl;

    // Run until cancellation token
    while (!this->detectionShouldCancel) {
        // Detect interaction with isCalibrating = false
        Interaction *interaction = this->detector->detectInteraction();
        // Handle interaction
        this->mouseHandler->handleInteraction(interaction);
        if (interaction != NULL) {
            // Free interaction
            this->detector->freeInteraction(interaction);
        }
#ifdef VIRTUALMONITOR_TEST_SNAPSHOT
        break;
#endif
    }

    this->detector->stop();
#endif
}

/*** Calibration ***/

/*
 * Starts detection, spawns calibrationThread
 *  to continuously read Kinect data and look for interactions
 */
int VirtualMonitorFrame::startCalibration() {
    if (this->calibrationFrame != NULL) {
        delete this->calibrationFrame;
    }
    this->calibrationFrame = new CalibrationFrame(CALIBRATION_ROWS, CALIBRATION_COLS);
    this->calibrationFrame->Show(true);
    
    this->calibrationThread = new VirtualMonitorCalibrationThread(this);
    if (this->calibrationThread->Create() != wxTHREAD_NO_ERROR) {
        return -1;
    }
    if (this->calibrationThread->Run() != wxTHREAD_NO_ERROR) {
        return -1;
    }
    
    return 0;
}

int VirtualMonitorFrame::stopCalibration() {
    //this->calibrationThread->join();
    this->calibrationFrame->Close();
    this->state = VirtualMonitorState::Paused;
    return 0;
}

int VirtualMonitorFrame::readCalibrationDataFromFile(Coord3D **calibrationPhysicalCoords, Coord2D **calibrationVirtualCoords, std::string calibrationDataFilename) {
    std::ifstream calibrationFile(calibrationDataFilename);
    if (!calibrationFile.is_open()) {
        std::cout << "VirtualMonitor: Could not read calibration data." << std::endl;
        return -1;
    }

    int calibrationIndex = 0;
    int physicalX, physicalY, virtualX, virtualY;
    float physicalZ;
    while (calibrationFile >> physicalX >> physicalY >> physicalZ >> virtualX >> virtualY) {
        Coord3D *physicalCoord = this->calibrationPhysicalCoords[calibrationIndex];
        physicalCoord->x = physicalX;
        physicalCoord->y = physicalY;
        physicalCoord->z = physicalZ;
        Coord2D *virtualCoord = this->calibrationVirtualCoords[calibrationIndex];
        virtualCoord->x = virtualX;
        virtualCoord->y = virtualY;
        calibrationIndex++;
    }

    calibrationFile.close();
    return 0;
}

int VirtualMonitorFrame::writeCalibrationDataToFile(Coord3D **calibrationPhysicalCoords, Coord2D **calibrationVirtualCoords, std::string calibrationDataFilename) {
    std::ofstream calibrationFile(calibrationDataFilename);
    if (!calibrationFile.is_open()) {
        std::cout << "VirtualMonitor: Could not write calibration data." << std::endl;
        return -1;
    }

    for (int calibrationIndex = 0; calibrationIndex < (CALIBRATION_ROWS * CALIBRATION_COLS); calibrationIndex++) {
        Coord3D *physicalCoord = this->calibrationPhysicalCoords[calibrationIndex];
        Coord2D *virtualCoord = this->calibrationVirtualCoords[calibrationIndex];
        calibrationFile << physicalCoord->x << " " << physicalCoord->y << " " << physicalCoord->z << " " << virtualCoord->x << " " << virtualCoord->y << std::endl;
    }


    calibrationFile.close();
    return 0;
}

/*  
 * Continuously reads Kinect data and looks for interactions and alerts main thread when they occur
 */
wxThread::ExitCode VirtualMonitorCalibrationThread::Entry() {
    // Parent frame
    VirtualMonitorFrame *parentFrame = (VirtualMonitorFrame *)this->m_parent;

    // Check for errors in starting detector
    if (parentFrame->detector->start() < 0) {
        return ExitCode(NULL);
    }

    std::cout << "Starting calibration..." << std::endl;

    // Go through all calibration points
    int calibrationIndex = 0;
    while (calibrationIndex < (CALIBRATION_ROWS * CALIBRATION_COLS)) {
        // Detect interaction with isCalibrating = true
        Interaction *interaction = parentFrame->detector->detectInteraction(true);
        // Determine whether this interaction was a click up
        bool isCalibrationTapComplete = parentFrame->calibrationHandler->handleInteraction(interaction);
        if (isCalibrationTapComplete) {
            // Set the virtual coords of the calibration
            parentFrame->calibrationFrame->getCurrentCalibrationPoint(parentFrame->calibrationVirtualCoords[calibrationIndex]);
            
            // Set the physical coords of the calibration
            parentFrame->calibrationPhysicalCoords[calibrationIndex]->x = interaction->physicalLocation->x;
            parentFrame->calibrationPhysicalCoords[calibrationIndex]->y = interaction->physicalLocation->y;
            parentFrame->calibrationPhysicalCoords[calibrationIndex]->z = interaction->physicalLocation->z;
            std::cout << "Calibration updating with x: " << interaction->physicalLocation->x << " y: " << interaction->physicalLocation->y << " z: " << interaction->physicalLocation->z << std::endl;

            // Notify UI thread of calibration update
            wxCommandEvent calibrationEvent(VIRTUALMONITOR_CALIBRATE_THREAD_UPDATE, wxID_ANY);
            calibrationEvent.SetInt(calibrationIndex);
            m_parent->AddPendingEvent(calibrationEvent);
            
            // Move on to next calibration point
            calibrationIndex++;
        }
        if (interaction != NULL) {
            // Free interaction
            parentFrame->detector->freeInteraction(interaction);
        }
    }

    // Write calibration to file
    parentFrame->writeCalibrationDataToFile(parentFrame->calibrationPhysicalCoords, parentFrame->calibrationVirtualCoords, CALIBRATION_DATA_FILENAME);

    parentFrame->detector->stop();
    
    return ExitCode(NULL);
}

