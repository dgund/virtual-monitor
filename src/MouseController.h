/*
    Virtual Monitor
    Transforms a projected computer screen into an intuitive touchscreen device.
    Copyright (C) 2018 Devin Gund (https://dgund.com)

    MouseController.h
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
