/*
    VirtualMonitor.cpp
    Main file for controlling the virtual monitor.
*/

#include "VirtualMonitor.h"

#include <iostream>

#include "InteractionDetector.h"
#include "InteractionHandler.h"
#include "unistd.h"

#define LABEL_START_DETECTION "Start Detection"
#define LABEL_STOP_DETECTION "Stop Detection"
#define LABEL_CALIBRATE "Calibrate"

// How many calibration rows and cols
#define CALIBRATION_ROWS 2
#define CALIBRATION_COLS 4

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
        // Set the virtual coords of the calibration
        this->calibrationFrame->getCurrentCalibrationPoint(this->calibrationVirtualCoords[calibrationIndex]);
        
        // Set the physical coords of the calibration
        Coord3D *physicalLocation = (Coord3D *)event.GetClientData();
        this->calibrationPhysicalCoords[calibrationIndex]->x = physicalLocation->x;
        this->calibrationPhysicalCoords[calibrationIndex]->y = physicalLocation->y;
        this->calibrationPhysicalCoords[calibrationIndex]->z = physicalLocation->z;
        std::cout << "Calibration updating with x: " << physicalLocation->x << " y: " << physicalLocation->y << " z: " << physicalLocation->z << std::endl;

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
    // Detects interactions with the virtual monitor from sensor data
    InteractionDetector *detector = new InteractionDetector();
    // Handles interactions with the virtual monitor
        // Determines click down and click up locations
        // Simulates clicks
    InteractionHandler *handler = new InteractionHandler();

#ifdef VIRTUALMONITOR_TEST_INPUTS
    // Detect interaction with isCalibrating = false, outputPPMData = true
    Interaction *interaction = detector->testDetectInteraction(false, true);
    // Handle interaction
    handler->handleInteraction(interaction);
    if (interaction != NULL) {
        // Free interaction
        detector->freeInteraction(interaction);
    }
#else
    // Pass in calibration data to be used by virtualManager
    detector->setCalibrationPoints(CALIBRATION_ROWS, CALIBRATION_COLS, this->calibrationPhysicalCoords, this->calibrationVirtualCoords);
    detector->setScreenVirtual(wxSystemSettings::GetMetric(wxSYS_SCREEN_Y), wxSystemSettings::GetMetric(wxSYS_SCREEN_X));

    // Check for errors in starting detector
    if (detector->start() < 0) {
        delete detector;
        delete handler;
        return;
    }

    std::cout << "Starting detection..." << std::endl;

    // Run until cancellation token
    while (!this->detectionShouldCancel) {
        // Detect interaction with isCalibrating = false
        Interaction *interaction = detector->detectInteraction();
        // Handle interaction
        handler->handleInteraction(interaction);
        if (interaction != NULL) {
            // Free interaction
            detector->freeInteraction(interaction);
        }
#ifdef VIRTUALMONITOR_TEST_SNAPSHOT
        break;
#endif
    }

    detector->stop();
#endif

    delete detector;
    delete handler;
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

/*  
 * Continuously reads Kinect data and looks for interactions and alerts main thread when they occur
 */
wxThread::ExitCode VirtualMonitorCalibrationThread::Entry() {
    // Detects interactions with the virtual monitor from sensor data
    InteractionDetector *detector = new InteractionDetector();
    // Handles interactions with the virtual monitor
        // Determines click down and click up locations
        // Updates calibrationCoords array
    InteractionHandler *handler = new InteractionHandler();

    // Check for errors in starting detector
    if (detector->start() < 0) {
        delete detector;
        delete handler;
        return ExitCode(NULL);
    }

    std::cout << "Starting calibration..." << std::endl;

    // Go through all calibration points
    int calibrationIndex = 0;
    while (calibrationIndex < (CALIBRATION_ROWS * CALIBRATION_COLS)) {
        // Detect interaction with isCalibrating = true
        Interaction *interaction = detector->detectInteraction(true);
        // Determine whether this interaction was a click up
        bool isCalibrationTapComplete = handler->handleInteraction(interaction);
        if (isCalibrationTapComplete) {
            // Notify UI thread of calibration update
            wxCommandEvent calibrationEvent(VIRTUALMONITOR_CALIBRATE_THREAD_UPDATE, wxID_ANY);
            calibrationEvent.SetClientData(interaction->physicalLocation);
            calibrationEvent.SetInt(calibrationIndex);
            m_parent->AddPendingEvent(calibrationEvent);
            
            // Move on to next calibration point
            calibrationIndex++;
        }
        if (interaction != NULL) {
            // Free interaction
            std::cout << "Calibration deleting interaction..." << std::endl;
            detector->freeInteraction(interaction);
            std::cout << "Calibration deleting interaction done..." << std::endl;
        }
    }

    detector->stop();

    delete detector;
    delete handler;
    
    return ExitCode(NULL);
}

