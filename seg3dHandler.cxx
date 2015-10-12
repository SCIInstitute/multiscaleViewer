#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "PracticalSocket.h"
#include "seg3dHandler.hpp"

#define SOCKET_VERBOSE_TEST

int seg3dHandler::sendSocketCommandToSeg3D(std::string command)
{
#ifdef SOCKET_VERBOSE_TEST
    std::cout << "SOCKET_SEND: " << command << std::endl;
#endif
    try {
      TCPSocket sock(mHost, mPort);
      sock.send(command.c_str(), command.length());
    } catch(SocketException &e) {
      std::cerr << e.what() << endl;
      return 1;
    }
    return 0;
}

int seg3dHandler::sendToSeg3D_openVolumeCommand(std::string filename)
{
  std::string cmd = "importlayer(filename=\"" + filename + "\", ";
  cmd += "importer=\"[Teem Importer]\")\r\n";
  return (sendSocketCommandToSeg3D(cmd));
}

int seg3dHandler::sendToSeg3D_openFileSeriesCommand(std::vector<std::string> filenames)
{
  std::string cmdPfx, cmdFiles, cmdSfx;
  cmdPfx  = "ImportSeries filenames='[";

  for (auto & element : filenames)
    cmdFiles += "[" + element + "],";
  cmdFiles = cmdFiles.substr(0, cmdFiles.size() - 1);

  cmdSfx  = "]' importer='[ITK FileSeries Importer]' ";
  cmdSfx += "mode='data' inputfiles_id='-1'";
  cmdSfx += "\r\n";
  return (sendSocketCommandToSeg3D(cmdPfx + cmdFiles + cmdSfx));
}

int seg3dHandler::determineCallToSeg3d(size_t objectIndex)
{
    if( mVolumes->isVolumeImageSeries(objectIndex) )
        return sendToSeg3D_openFileSeriesCommand(
            mVolumes->getImageSeriesListing(objectIndex));
    else
        return sendToSeg3D_openVolumeCommand(
            mVolumes->getVolFilenames(objectIndex));
}

int seg3dHandler::objectClickedCallback(size_t objectIndex)
{
    std::string openMessagePrefix;
    std::string filenameOfVolume;

    if( mVolumes->isVolumeImageSeries(objectIndex) )
    {
        filenameOfVolume = mVolumes->getVolFilenames(objectIndex);
        openMessagePrefix = "Opening image series: ";
    }
    else
    {
        filenameOfVolume = mVolumes->getVolFilenames(objectIndex);
        openMessagePrefix = "Opening single image/volume: ";
    }

    if( filenameOfVolume.length() != 0 )
    {
        std::cout << openMessagePrefix << filenameOfVolume;
        std::cout << " in Seg3D." << std::endl;
        return sendToSeg3D_openFileSeriesCommand(
        mVolumes->getImageSeriesListing(objectIndex));
    }
    else
        return -9;
}
