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
// how many calibration rows and cols
#define CAL_ROWS 3
#define CAL_COLS 2

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
    ID_DETECT = 1,
    ID_CALIBRATE = 2
};

/*
 * Event table for handling user interacting with controls
 */
wxBEGIN_EVENT_TABLE(VirtualMonitorFrame, wxFrame)
    EVT_MENU(ID_DETECT, VirtualMonitorFrame::OnDetect)
    EVT_MENU(ID_CALIBRATE, VirtualMonitorFrame::OnCalibrate)
    EVT_MENU(wxID_EXIT, VirtualMonitorFrame::OnExit)
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
    this->detectButton = new wxButton(this->panel, ID_DETECT, wxT(LABEL_START_DETECTION), wxPoint(10, 10));
    Connect(ID_DETECT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(VirtualMonitorFrame::OnDetect));
    this->detectButton->Show(true);

    this->calibrateButton = new wxButton(this->panel, ID_CALIBRATE, wxT(LABEL_CALIBRATE), wxPoint(10, 40));
    Connect(ID_CALIBRATE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(VirtualMonitorFrame::OnCalibrate));
    this->calibrateButton->Show(true);

    // Show text that might be used to help user
    // TODO currently filler text
    this->textLabel = new wxStaticText(this->panel, 0, wxT("Text"), wxPoint(10, 70));
    this->textLabel->Show(true);

    panel->Fit();

    // Initialize array of physical calibration data
    this->calibrationCoords = new Coord3D*[NUM_ROWS*NUM_COLS];
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
    delete this->calibrate;
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
        // Open a new calibration window
        this->calibrate = new Calibrate(wxT("Calibrate"), CAL_ROWS, CAL_COLS, 20);
        this->calibrate->Show(true);
        // Start calibrating
        this->startCalibration();
        break;
    }
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
    Interaction *interaction = detector->testDetectInteraction(true);
    if (interaction != NULL) {
        // Handle interaction
        handler->handleInteraction(interaction);
        // Free interaction
        detector->freeInteraction(interaction);
    }
#else
    // Pass in calibration data to be used by virtualManager
    // detector->virtualManager->setCalibrationPoints(rows, cols, xMax, yMax, calibrationCoords array **)

    // Check for errors in starting detector
    if (detector->start() < 0) {
        delete detector;
        delete handler;
        return;
    }

    // Run until cancellation token
    while (!this->detectionShouldCancel) {
        // Detect interaction with isCalibrating = false
        Interaction *interaction = detector->detectInteraction(false);
        if (interaction != NULL) {
            // Handle interaction
            handler->handleInteraction(interaction);
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
    // Start interaction detection/handling on new thread
    this->calibrationThread = std::thread(&VirtualMonitorFrame::calibrationThreadFn, this);
    return 0;
}

/*  
 * Continuously reads Kinect data and looks for interactions
 */
void VirtualMonitorFrame::calibrationThreadFn() {
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
        return;
    }

    // Go through all calibration points
    int calibrationIndex = 0;
    while (calibrationIndex < (CAL_ROWS * CAL_COLS)) {
        // Detect interaction with isCalibrating = true
        Interaction *interaction = detector->detectInteraction(true);
        if (interaction != NULL) {
            // Determine whether this interaction was a click up
            bool calTap = handler->handleInteraction(interaction);
            if (calTap) {
                this->calibrationCoords[calibrationIndex] = interaction->physicalLocation;
                this->calibrate->NextButton();
                calibrationIndex++;
            }
            // Free interaction
            detector->freeInteraction(interaction);
        }
    }

    detector->stop();

    delete detector;
    delete handler;
}
