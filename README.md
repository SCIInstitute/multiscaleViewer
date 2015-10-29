# multiscaleViewer
Viewer for Multiple Volumes of Varying Scale

Current version: 0.9

The Multi-Scale Viewer can be used (in conjunction with Seg3D) to view an overall view of a set of data volumes. The Viewer will render a bounding box in 3D for each data volume in the set. The position and origin of each volume is rendered to scale to help the user understand the spatial relationship between volumes. This scaling rule holds only for volumes whose largest dimension is at least 1% of the largest dimension of all volumes in the entire set. This rule ensures that a volume bounding box is not rendered too small to be seen.
The Multi-Scale Viewer is currently only supported on Mac OS 10.10 and above.

**Installation Requriements**

SCI software Seg3D must be installed. See http://sci.utah.edu/cibc-software/seg3d.html for details.
The VTK library version 6.3.0 must also be installed. The recommended method of installation is homebrew. If homebrew is not installed, use the command:

*ruby -e “$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)”*

and then the homebrew command:

*brew install homebrew/science/vtk –with-qt*

to install VTK 6.3.0 (this process will take a little while). If a manual install is desired (or if homebrew won't install version 6.3.0), see http://www.vtk.org/download/ for details.

**Volume Set Configuration**

The full set of volumes is defined in a single text file named *load_volumes.txt*, which resides in the same directory as the viewer executable. This is an ASCII text file with one data field per line. In this file, each volume is described by 6 lines. The format of this file is as follows:

Line # | Field Description
-------|------------------
1 | Contains the text "VOLUME" only
2 | Contains an ABSOLUTE file path to the file or series of files to be displayed with this volume. This can be an individual nrrd file, or a file filter pattern to specify a series of files that will be opened and stacked in order to create the volume. This would specify and individual file: */Users/Joe/path/to/singleVolumeFile.nrrd*, and this would specify a series of files: */Users/Joe/path/to/ImageSeries_00***.tiff*
3 | Origin of the volume in 3 dimensions. 3 floating-point values are provided with a space between them.
4 | X and Y dimensions of a cross section of the volume in the XY plane, values floating-point with a space between them.
5 | Number of slices in the Z dimension (single floating point value)
6 | Spacing between slices in the Z axis (single floating point value)


**Running the Application**

Once the *load_volumes.txt* file has been configured with the desired volume settings, the Viewer can be started. First, run Seg3D software by running it from a Terminal window with the specified socket port number of 9999. An example path might look like this: */Applications/Seg3D2.app/Contents/MacOS/Seg3D2 --socket=9999*. The Viewer is then started by running *./multiscale* (or similar path) in a Terminal window. 

**Using the Application**

By left-clicking & dragging, the 3D view of the volume bounding boxes changes by rotating the camera. Holding the shift button pans the camera. Scrolling the mouse wheel (or two-finger drag) zooms the camera. Left-flicking anywhere on the bounding box of a volume will print its detail in the Terminal window, and then open that volume in Seg3D. If a “connection refused” message appears, then it should be verified that Seg3D is running with the *--socket=9999* option. Press 'q' in the viewer to quit the application.
