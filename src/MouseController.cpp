/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    MouseController.cpp
    Interfaces mouse controls with the OS.

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

#include "MouseController.h"

#include <iostream>

#ifdef _WIN32

#elif __APPLE__
#include <ApplicationServices/ApplicationServices.h>

#elif __linux__

#elif __unix__

#elif defined(_POSIX_VERSION)

#else
#error "MouseController: Unknown compiler."
#endif

#define CLICK_DELAY_MS 100000

namespace virtualMonitor {

MouseController::MouseController() {}

MouseController::~MouseController() {}

int MouseController::move(Coord2D *location) {
#ifdef _WIN32
    return -1;

#elif __APPLE__
    CGPoint target = CGPointMake(location->x, location->y);
    CGEventRef event = CGEventCreateMouseEvent(
        NULL, kCGEventMouseMoved,
        target, kCGMouseButtonLeft
    );
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    return 0;

#elif __linux__
    return -1;

#elif __unix__
    return -1;

#elif defined(_POSIX_VERSION)
    return -1;

#else
#error "MouseController: Unknown compiler."
#endif
}

int MouseController::leftMouseDown(Coord2D *location) {
#ifdef _WIN32
    return -1;

#elif __APPLE__
    CGPoint target = CGPointMake(location->x, location->y);
    CGEventRef event = CGEventCreateMouseEvent(
        NULL, kCGEventLeftMouseDown,
        target, kCGMouseButtonLeft
    );
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    return 0;

#elif __linux__
    return -1;

#elif __unix__
    return -1;

#elif defined(_POSIX_VERSION)
    return -1;

#else
#error "MouseController: Unknown compiler."
#endif
}

int MouseController::leftMouseUp(Coord2D *location) {
#ifdef _WIN32
    return -1;

#elif __APPLE__
    CGPoint target = CGPointMake(location->x, location->y);
    CGEventRef event = CGEventCreateMouseEvent(
        NULL, kCGEventLeftMouseUp,
        target, kCGMouseButtonLeft
    );
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    return 0;

#elif __linux__
    return -1;

#elif __unix__
    return -1;

#elif defined(_POSIX_VERSION)
    return -1;

#else
#error "MouseController: Unknown compiler."
#endif
}

int MouseController::rightMouseDown(Coord2D *location) {
#ifdef _WIN32
    return -1;

#elif __APPLE__
    CGPoint target = CGPointMake(location->x, location->y);
    CGEventRef event = CGEventCreateMouseEvent(
        NULL, kCGEventRightMouseDown,
        target, kCGMouseButtonRight
    );
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    return 0;

#elif __linux__
    return -1;

#elif __unix__
    return -1;

#elif defined(_POSIX_VERSION)
    return -1;

#else
#error "MouseController: Unknown compiler."
#endif
}

int MouseController::rightMouseUp(Coord2D *location) {
#ifdef _WIN32
    return -1;

#elif __APPLE__
    CGPoint target = CGPointMake(location->x, location->y);
    CGEventRef event = CGEventCreateMouseEvent(
        NULL, kCGEventRightMouseUp,
        target, kCGMouseButtonRight
    );
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
    return 0;

#elif __linux__
    return -1;

#elif __unix__
    return -1;

#elif defined(_POSIX_VERSION)
    return -1;

#else
#error "MouseController: Unknown compiler."
#endif
}

int MouseController::leftClick(Coord2D *location) {
#ifdef _WIN32
    return -1;

#elif __APPLE__
    this->leftMouseDown(location);
    usleep(CLICK_DELAY_MS);
    this->leftMouseUp(location);
    return 0;

#elif __linux__
    return -1;

#elif __unix__
    return -1;

#elif defined(_POSIX_VERSION)
    return -1;

#else
#error "MouseController: Unknown compiler."
#endif
}

int MouseController::rightClick(Coord2D *location) {
#ifdef _WIN32
    return -1;

#elif __APPLE__
    this->rightMouseDown(location);
    usleep(CLICK_DELAY_MS);
    this->rightMouseUp(location);
    return 0;

#elif __linux__
    return -1;

#elif __unix__
    return -1;

#elif defined(_POSIX_VERSION)
    return -1;

#else
#error "MouseController: Unknown compiler."
#endif
}

} /* namespace virtualMonitor */
