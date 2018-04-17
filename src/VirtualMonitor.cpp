/*
    VirtualMonitor.cpp
    Main file for controlling the virtual monitor.
*/

#include "VirtualMonitor.h"

#include <iostream>

#include "InteractionDetector.h"
#include "InteractionHandler.h"

#define LABEL_START_DETECTION "Start Detection"
#define LABEL_STOP_DETECTION "Stop Detection"
#define LABEL_CALIBRATE "Calibrate"

using namespace virtualMonitor;

// VirtualMonitorApp

wxIMPLEMENT_APP(VirtualMonitorApp);

bool VirtualMonitorApp::OnInit() {
    VirtualMonitorFrame *frame = new VirtualMonitorFrame();
    frame->Show(true);
    return true;
}

// VirtualMonitorFrame

enum {
    ID_DETECT = 1,
    ID_CALIBRATE = 2
};

wxBEGIN_EVENT_TABLE(VirtualMonitorFrame, wxFrame)
    EVT_MENU(ID_DETECT, VirtualMonitorFrame::OnDetect)
    EVT_MENU(ID_CALIBRATE, VirtualMonitorFrame::OnCalibrate)
    EVT_MENU(wxID_EXIT, VirtualMonitorFrame::OnExit)
wxEND_EVENT_TABLE()

VirtualMonitorFrame::VirtualMonitorFrame() : wxFrame(NULL, wxID_ANY, wxT("Virtual Monitor"), wxDefaultPosition, wxSize(180, 120)) {
    this->state = VirtualMonitorState::Paused;

    this->panel = new wxPanel(this, wxID_ANY);
    this->panel->Show(true);

    this->detectButton = new wxButton(this->panel, ID_DETECT, wxT(LABEL_START_DETECTION), wxPoint(10, 10));
    Connect(ID_DETECT, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(VirtualMonitorFrame::OnDetect));
    this->detectButton->Show(true);

    this->calibrateButton = new wxButton(this->panel, ID_CALIBRATE, wxT(LABEL_CALIBRATE), wxPoint(10, 40));
    Connect(ID_CALIBRATE, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(VirtualMonitorFrame::OnCalibrate));
    this->calibrateButton->Show(true);

    this->textLabel = new wxStaticText(this->panel, 0, wxT("Text"), wxPoint(10, 70));
    this->textLabel->Show(true);

    panel->Fit();
}

VirtualMonitorFrame::~VirtualMonitorFrame() {
    delete this->textLabel;
    delete this->calibrateButton;
    delete this->detectButton;
    delete this->panel;
}

void VirtualMonitorFrame::OnDetect(wxCommandEvent& event) {
    switch (this->state) {
    case VirtualMonitorState::Calibrating:
        return;
    case VirtualMonitorState::Detecting:
        // Stop detection
        this->stopDetection();
        this->state = VirtualMonitorState::Paused;
        break;
    case VirtualMonitorState::Paused:
        // Start detecting
        this->startDetection();
        this->state = VirtualMonitorState::Detecting;
        break;
    }

    this->detectButton->SetLabel(this->state == VirtualMonitorState::Detecting ? LABEL_STOP_DETECTION : LABEL_START_DETECTION);
}

void VirtualMonitorFrame::OnCalibrate(wxCommandEvent& event) {
    switch (this->state) {
    case VirtualMonitorState::Calibrating:
        // Already calibrating
        return;
    case VirtualMonitorState::Detecting:
        // Stop detection
        this->stopDetection();
        this->state = VirtualMonitorState::Paused;
        // continue
    case VirtualMonitorState::Paused:
        // Start calibration
        this->state = VirtualMonitorState::Calibrating;
        break;
    }
}

void VirtualMonitorFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

int VirtualMonitorFrame::startDetection() {
    // Reset cancellation token
    this->detectionShouldCancel = false;
    // Start interaction detection/handling on new thread
    this->detectionThread = std::thread(&VirtualMonitorFrame::detectionThreadFn, this);
    return 0;
}

int VirtualMonitorFrame::stopDetection() {
    this->detectionShouldCancel = true;
    this->detectionThread.join();
    return 0;
}

void VirtualMonitorFrame::detectionThreadFn() {
    InteractionDetector *detector = new InteractionDetector();
    InteractionHandler *handler = new InteractionHandler();

    // TODO set calibration

    detector->start();

    // Run until cancellation token
    while (!this->detectionShouldCancel) {
        // Detect interaction
        Interaction *interaction = detector->detectInteraction();
        if (interaction != NULL) {
            // Handle interaction
            handler->handleInteraction(interaction);
            // Free interaction
            detector->freeInteraction(interaction);
        }
    }

    detector->stop();

    delete detector;
    delete handler;
}
