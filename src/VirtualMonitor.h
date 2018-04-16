/*
    VirtualMonitor.h
    Main file for controlling the virtual monitor.
*/

#ifndef VIRTUALMONITOR_H
#define VIRTUALMONITOR_H

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

#endif /* VIRTUALMONITOR_H */
