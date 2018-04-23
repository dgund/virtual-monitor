/*
    MouseController.h
    Interfaces mouse controls with the OS.
*/

#ifndef MOUSECONTROLLER_H
#define MOUSECONTROLLER_H

#include "Location.h"

namespace virtualMonitor {

class MouseController {
public:
    MouseController();
    virtual ~MouseController();
    virtual int move(Coord2D *location);
    virtual int leftMouseDown(Coord2D *location);
    virtual int rightMouseDown(Coord2D *location);
    virtual int leftMouseUp(Coord2D *location);
    virtual int rightMouseUp(Coord2D *location);
    virtual int leftClick(Coord2D *location);
    virtual int rightClick(Coord2D *location);
};

} /* namespace virtualMonitor */

#endif /* MOUSECONTROLLER_H */
