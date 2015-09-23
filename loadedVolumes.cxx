#include <vector>
#include <string>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include "loadedVolumes.hpp"

loadedVolumes::loadedVolumes(std::string sourceFilename) :
    mNumVolumes(0), mFilename(sourceFilename)
{
    readVolumesDescriptionFile();
}

loadedVolumes::~loadedVolumes(void) {}

size_t loadedVolumes::readVolumesDescriptionFile(void)
{
    std::ifstream file(mFilename);
    std::string line;
    std::string tmpString;
    int totalLinesRead = 0;

    if( file.good() )
    {
        FileReadState fileState = parseHeader;

        while(std::getline(file, line))
        {
            std::stringstream linestream(line);

            switch (fileState)
            {
                case parseHeader:
                    std::getline(linestream, tmpString, '\n');
                    if( tmpString.compare("VOLUME") != 0 )
                    {
                        std::string msg = "VOLUME header not found on line ";
                        msg += std::to_string(totalLinesRead);
                        throw std::string(msg);
                    }
                    fileState = parseFile;
                    break;

                case parseFile:
                    std::getline(linestream, tmpString, '\n');
                    mFilename = tmpString;
                    fileState = parseOrigin;
                    break;

                case parseOrigin:
                    std::array<float, 3> tmpOrigin;
                    std::getline(linestream, tmpString, ' ');
                    tmpOrigin[0] = strtof(tmpString.c_str(), NULL);
                    std::getline(linestream, tmpString, ' ');
                    tmpOrigin[1] = strtof(tmpString.c_str(), NULL);
                    std::getline(linestream, tmpString, '\n');
                    tmpOrigin[2] = strtof(tmpString.c_str(), NULL);
                    mOrigin.push_back(tmpOrigin);
                    fileState = parseXYresolution;
                    break;

                case parseXYresolution:
                    std::array<float, 2> tmpRes;
                    std::getline(linestream, tmpString, ' ');
                    tmpRes[0] = strtof(tmpString.c_str(), NULL);
                    std::getline(linestream, tmpString, '\n');
                    tmpRes[1] = strtof(tmpString.c_str(), NULL);
                    mXYresolution.push_back(tmpRes);
                    fileState = parseSlices;
                    break;

                case parseSlices:
                    std::getline(linestream, tmpString, '\n');
                    mZslices.push_back(strtof(tmpString.c_str(), NULL));
                    fileState = parseSliceThickness;
                    break;

                case parseSliceThickness:
                    std::getline(linestream, tmpString, '\n');
                    mSliceThickness.push_back(strtof(tmpString.c_str(), NULL));
                    fileState = parseHeader;
                    mNumVolumes++;
                    break;

                default:
                    break;
            }
            totalLinesRead++;
        }
        if( totalLinesRead%numLinesPerVolume != 0 || totalLinesRead == 0 )
        {
            throw std::string("Error in reading volume file.");
        }
    }
    else
    {
        throw std::string("Cannot open file for approved drives list.");
    }
    return mNumVolumes;
}
