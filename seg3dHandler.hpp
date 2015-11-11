#ifndef __SEG_3D_HANDLER__
#define __SEG_3D_HANDLER__

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "PracticalSocket.h"
#include "loadedVolumes.hpp"

/*seg3dHandler class
 * Gene Payne
 * This class provides the multiscale application with a way to signal Seg3D
 * software to open a data volume that is clicked on by the user in this
 * application. Multiscale sends a command to Seg3D (which is assumed to be
 * running on localhost) via a socket connection on the port specified in this
 * file. The Seg3D software must be opened on the same machine, and listening
 * on the correct port, by using a command similar to:
 * /Applications/Seg3D2.app/Contents/MacOS/Seg3D2 --socket=9999
 *
 * The "PracticalSocket" source code is used for socket communication with
 * Seg3D. This source code was released under the GNU license. See that file's
 * header for more information.
*/

class seg3dHandler
{
    public:
	    //Constructor for seg3dHandler with pointer to loadedVolumes object
	    // which contains the details of the volumes that are available
	    seg3dHandler(loadedVolumes* volumes) : mVolumes(volumes) {}
	    virtual ~seg3dHandler(void) {}
	    //Sends a command to Seg3D to open a single volume file (e.g. nrrd)
	    // filename contains the path to the file.
        int sendToSeg3D_openVolumeCommand(std::string filename);
        //Sends a command to Seg3D to open a single image file (e.g. tiff),
        // rather than an image series. filename contains the path to the file.
        int sendToSeg3D_openOneFileCommand(std::string filename);
	    //Sends a command to Seg3D to open a series of images that constitute
        // a data volume. filenames contains a filter string that specified
        // the file pattern that will capture all files in the volume
        // (e.g. /path/to/files_00*.tiff)
        int sendToSeg3D_openFileSeriesCommand(std::vector<std::string> filenames);
        //The software that handles selection of the volume (such as a mouse
        // click) will be given a pointer to this callback. The parameter
        // objectIndex is the index of the volume selected (where loadedVolumes
        // object contains all volumes)
        int objectClickedCallback(size_t objectIndex);

    private:
        //Determines the syntax of the python command to send to Seg3D, based
        // on the type of file/series to open. objectIndex is the volume
        // index in the loadedVolumes object.
        int determineCallToSeg3d(size_t objectIndex);
        //Send a python command to Seg3D. command is the exact string to send
        int sendSocketCommandToSeg3D(std::string command);

        std::string mHost = "localhost";
        int mPort = 9999;
        loadedVolumes* mVolumes;
};

#endif //#ifndef __SEG_3D_HANDLER__
