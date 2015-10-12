#include <vector>
#include <string>
#include <cstring>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include "loadedVolumes.hpp"

loadedVolumes::loadedVolumes(std::string sourceFilename, std::string offset) :
    mNumVolumes(0), mSourceFilename(sourceFilename), mPathOffset(offset)
{
    readVolumesDescriptionFile();
}

loadedVolumes::loadedVolumes(const loadedVolumes& src)
{
	copyFrom(src);
}

loadedVolumes::~loadedVolumes(void) {}

loadedVolumes& loadedVolumes::operator=(const loadedVolumes& rhs)
{
	if( &rhs == this )
		return *this;
    copyFrom(rhs);
    return *this;
}

void loadedVolumes::populateImageSeriesListFromFileFilterString(
    std::string filter, size_t indexIntoListsOfFiles)
{
	char tmpBuff[MAX_SINGLE_FILENAME_SIZE];
	std::string listingCmd = "ls -1 " + filter;
    FILE* stream = popen(listingCmd.c_str(), "r");
    std::vector<std::string> tmpListing;

    if( stream ) {
        while( ! feof(stream) ) {
            if( fgets(tmpBuff, MAX_SINGLE_FILENAME_SIZE, stream) != NULL ) {
                std::string tmpString = removeNewlineAndConvertToString(tmpBuff);
                tmpListing.push_back(tmpString);
            }
        }
        mImageSeriesFilenames.push_back(tmpListing);
        pclose(stream);
    } else {
        throw std::string("Cannot open pipe to get image series listing.");
    }

}

void loadedVolumes::addOffsetToPath(std::string& filePathInput)
{
    if( mPathOffset.size() != 0 )
    	filePathInput = mPathOffset + filePathInput;
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
                    addOffsetToPath(tmpString);
                    if( doesPathRepresentImageSeries(tmpString) )
                    {
                        mIsImageSeries.push_back(true);
                        try {
                            populateImageSeriesListFromFileFilterString(
                                tmpString, mNumVolumes);
                        } catch(std::string &e) {
                          std::cerr << e << std::endl;
                        }
std::cout << "Image series";
                    }
                    else
                    {
                    	mIsImageSeries.push_back(false);
                    	std::vector<std::string> tmpV;
                    	tmpV.push_back(tmpString);
                    	mImageSeriesFilenames.push_back(tmpV);
std::cout << "Single volume";
                    }
                    mVolFilename.push_back(tmpString);
                    fileState = parseOrigin;
std::cout << "(" << mVolFilename[mNumVolumes] << " / ";
for (auto& element : mImageSeriesFilenames[mNumVolumes])
	std::cout << element << " ";
std::cout << ") at index " << mNumVolumes << "." << std::endl;
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

void loadedVolumes::copyFrom(const loadedVolumes& src)
{
    mNumVolumes = src.mNumVolumes;
    mSourceFilename = src.mSourceFilename;
    mVolFilename = src.mVolFilename;
    mImageSeriesFilenames = src.mImageSeriesFilenames;
    mOrigin = src.mOrigin;
    mXYresolution = src.mXYresolution;
    mZslices = src.mZslices;
    mSliceThickness = src.mSliceThickness;
    mIsImageSeries = src.mIsImageSeries;
}
