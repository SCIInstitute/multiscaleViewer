#ifndef __SEG_3D_HANDLER__
#define __SEG_3D_HANDLER__

#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "PracticalSocket.h"
#include "loadedVolumes.hpp"

class seg3dHandler
{
    public:
	    seg3dHandler(loadedVolumes* volumes) : mVolumes(volumes) {}
	    ~seg3dHandler();
        int sendToSeg3D_openVolumeCommand(std::string filename);
        int sendToSeg3D_openFileSeriesCommand(std::vector<std::string> filenames);
        int objectClickedCallback(size_t objectIndex);

    private:
        int determineCallToSeg3d(size_t objectIndex);
        int sendSocketCommandToSeg3D(std::string command);

        std::string mHost = "localhost";
        int mPort = 9999;
        loadedVolumes* mVolumes;
};

#endif //#ifndef __SEG_3D_HANDLER__
