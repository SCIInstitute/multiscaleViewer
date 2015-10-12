#include <vector>
#include <string>
#include <cstring>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include "loadedVolumes.hpp"

loadedVolumes::loadedVolumes(std::string sourceFilename) :
    mNumVolumes(0), mSourceFilename(sourceFilename)
{
    readVolumesDescriptionFile();
}

loadedVolumes::~loadedVolumes(void) {}

void loadedVolumes::populateImageSeriesListFromFileFilterString(
    std::string filter, size_t indexIntoListsOfFiles)
{
	char tmpBuff[MAX_SINGLE_FILENAME_SIZE];
	std::string listingCmd = "ls -1 " + filter;
    FILE* stream = popen(listingCmd.c_str(), "r");
    if( stream ) {
    	mImageSeriesFilenames[indexIntoListsOfFiles].clear();
        while( ! feof(stream) ) {
            if( fgets(tmpBuff, MAX_SINGLE_FILENAME_SIZE, stream) != NULL ) {
                std::string tmpString = removeNewlineAndConvertToString(tmpBuff);
                mImageSeriesFilenames[indexIntoListsOfFiles].push_back(tmpString);
            }
        }
        pclose(stream);
    } else {
        throw std::string("Cannot open pipe to get image series listing.");
    }

}

std::string loadedVolumes::removeNewlineAndConvertToString(char* buffer)
{
    size_t ln = strlen(buffer) - 1;
    if( buffer[ln] == '\n' )
        buffer[ln] = '\0';
    std::string tmpString = buffer;
    return tmpString;
}

bool doesPathRepresentImageSeries(std::string filePath)
{
    std::size_t found = filePath.find('*');
    if( found != std::string::npos )
        return true;
    else
    	return false;
}

size_t loadedVolumes::readVolumesDescriptionFile(void)
{
    std::ifstream file(mSourceFilename);
    std::string line;
    std::string tmpString;
    int totalLinesRead = 0;
    std::size_t found;

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
                    if( doesPathRepresentImageSeries(tmpString) )
                    {
                        mIsImageSeries[mNumVolumes] = true;
                        try {
                            populateImageSeriesListFromFileFilterString(
                                tmpString, mNumVolumes);
                        } catch(std::string &e) {
                          std::cerr << e << std::endl;
                        }
                    }
                    else
                    {
                    	mIsImageSeries[mNumVolumes] = false;
                    	mImageSeriesFilenames[mNumVolumes].push_back(tmpString);
                    }
                    mVolFilename.push_back(tmpString);
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
