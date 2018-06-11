# Virtual Monitor

Transforms a projected computer screen into an intuitive touchscreen device.

Presentations and collaborative work are difficult on small computer screens, and while projectors can increase the size and visibility of screens, meaningful interaction must still be done through the computer. Virtual Monitor solves this problem by transforming any projected computer screen into an intuitive touchscreen device. Interactions (i.e. tapping and dragging) are performed directly on the projected screen. Unlike other "smart projector" products, which require costly and fixed installation, Virtual Monitor is designed to use depth data from a [Kinect](https://en.wikipedia.org/wiki/Kinect) and support any external projector hardware with flexible setup and placement. This software was developed as a senior capstone project at [Carnegie Mellon University](https://www.cmu.edu).

## Software Architecture

The software architecture is organized by functional area: interfaces, calibration, interaction detection, interaction handling. A full breakdown of ownership hierarchy and organization of the software classes is provided below:

- **VirtualMonitor**: main interface and top-level thread control
    - **Calibration**, **Interaction**, **Location**: library headers
    - **CalibrationFrame**: calibration interface
    - **InteractionDetector**: detects interation location
        - **KinectReader**: interfaces with Kinect to read depth data
        - **PhysicalManager**: detects interaction location in physical (3D) space
        - **VirtualManager**: converts interaction location to virtual (2D) space
    - **InteractionHandler**, **CalibrationInteractionHandler**, **MouseInteractionHandler**: handles interactions
        - **MouseController**: interfaces with operating system for mouse control

## Dependencies

- [libfreenect2](https://github.com/OpenKinect/libfreenect2): drivers for Kinect for Windows v2 devices
- [wxWidgets](https://www.wxwidgets.org): cross-platform GUI library

## Usage

After installing the dependencies, the project may be compiled using the included [Makefile](Makefile) and run from the executable at `./bin/VirtualMonitor`. Simply calibrate for the projected computer screen and interact by tapping and dragging. The project was designed to be cross-platform, but MouseController currently only includes drivers for macOS.

## Project Details

[ECE Design Experience](https://www.ece.cmu.edu/courses/items/18500.html) (18-500) is the senior capstone project course for [Electrical & Computer Engineering](https://www.ece.cmu.edu) at Carnegie Mellon University where students design, develop, and present engineering projects. I devised the Virtual Monitor concept and developed nearly all of software (see the [contribution history](https://github.com/dgund/virtual-monitor/graphs/contributors)). The full capstone project team was:

- Devin Gund (VirtualMonitor, InteractionDetector, KinectReader, PhysicalManager, InteractionHandler, MouseController, user interfaces, etc.)
- Cayla Wanderman-Milne (VirtualManager)
- Isabella Brahm (external testing)

This project is free and open-source software under the GNU General Public License v3.0.
