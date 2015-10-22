#include <vector>
#include <string>
#include <cstring>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "loadedVolumes.hpp"

const bool VerboseVolumeFileOutput = false;

loadedVolumes::loadedVolumes(std::string sourceFilename, std::string offset) :
    mNumVolumes(0), mSourceFilename(sourceFilename), mPathOffset(offset)
{
    readVolumesDescriptionFile();
    scaleVolumesThatAreTooSmallToBeVisible();
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
                    if( tmpString.substr(0, 1).compare("#") == 0 )
                        continue;
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
                    if( tmpString.substr(0, 1).compare("#") == 0 )
                        continue;
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
                        if( VerboseVolumeFileOutput )
                            std::cout << "Image series";
                    }
                    else
                    {
                    	mIsImageSeries.push_back(false);
                    	std::vector<std::string> tmpV;
                    	tmpV.push_back(tmpString);
                    	mImageSeriesFilenames.push_back(tmpV);
                        if( VerboseVolumeFileOutput )
                            std::cout << "Single volume";
                    }
                    mVolFilename.push_back(tmpString);
                    fileState = parseOrigin;
                    if( VerboseVolumeFileOutput )
                    {
                        std::cout << "([" << mVolFilename[mNumVolumes]
                            << "]" << std::endl;
                        for (auto& element : mImageSeriesFilenames[mNumVolumes])
                            std::cout << element << std::endl;
                        std::cout << ") at index " << mNumVolumes
                            << "." << std::endl;
                    }
                    break;

                case parseOrigin:
                    std::array<float, 3> tmpOrigin;
                    std::getline(linestream, tmpString, ' ');
                    if( tmpString.substr(0, 1).compare("#") == 0 )
                        continue;
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
                    if( tmpString.substr(0, 1).compare("#") == 0 )
                        continue;
                    tmpRes[0] = strtof(tmpString.c_str(), NULL);
                    std::getline(linestream, tmpString, '\n');
                    tmpRes[1] = strtof(tmpString.c_str(), NULL);
                    mXYresolution.push_back(tmpRes);
                    fileState = parseSlices;
                    break;

                case parseSlices:
                    std::getline(linestream, tmpString, '\n');
                    if( tmpString.substr(0, 1).compare("#") == 0 )
                        continue;
                    mZslices.push_back(strtof(tmpString.c_str(), NULL));
                    fileState = parseSliceThickness;
                    break;

                case parseSliceThickness:
                    std::getline(linestream, tmpString, '\n');
                    if( tmpString.substr(0, 1).compare("#") == 0 )
                        continue;
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

void loadedVolumes::scaleVolumesThatAreTooSmallToBeVisible(void)
{
	float tmpDim[3];

    findLargestSingleDimensionInVolumeSet();
    for (unsigned int i = 0; i < mNumVolumes; ++i)
    {
    	getThreeDimsOfThisVolume(i, tmpDim);
        if( getNumAxesThatAreTooSmall(tmpDim) == ALL_THREE_AXES )
        {
            scaleUpAnIndividualVolume(i, tmpDim);
        }
    }
}

void loadedVolumes::scaleUpAnIndividualVolume(unsigned int volIndex,
		                       float (&tmpDim)[THREE_DIMENSIONS])
{
    float largestDimInThisVolume = *std::max_element(tmpDim,
        tmpDim + THREE_DIMENSIONS);
	float scaleUpFactor = mSmallestAcceptableSizeRatio
        / (largestDimInThisVolume / mLargestSingleDim);
	for (unsigned int j = 0; j < TWO_DIMENSIONS; ++j)
		mXYresolution[volIndex][j] = tmpDim[j] * scaleUpFactor;
    mSliceThickness[volIndex] = tmpDim[Z_AXIS] * scaleUpFactor
        / mZslices[volIndex];
}

void loadedVolumes::getThreeDimsOfThisVolume(unsigned int volIndex,
		                                     float (&tmpDim)[THREE_DIMENSIONS])
{
    tmpDim[X_AXIS] = mXYresolution[volIndex][X_AXIS];
    tmpDim[Y_AXIS] = mXYresolution[volIndex][Y_AXIS];
    tmpDim[Z_AXIS] = mSliceThickness[volIndex] * mZslices[volIndex];
}

unsigned int loadedVolumes::getNumAxesThatAreTooSmall(
    float (&tmpDim)[THREE_DIMENSIONS])
{
	unsigned int count = 0;
    count += checkIfSingleDimIsTooSmall(tmpDim[X_AXIS]);
    count += checkIfSingleDimIsTooSmall(tmpDim[Y_AXIS]);
    count += checkIfSingleDimIsTooSmall(tmpDim[Z_AXIS]);
    return count;
}

unsigned int loadedVolumes::checkIfSingleDimIsTooSmall(float dim)
{
    if( dim < (mSmallestAcceptableSizeRatio * mLargestSingleDim) )
        return 1;
    else
    	return 0;
}

void loadedVolumes::findLargestSingleDimensionInVolumeSet(void)
{
	float largestSingleDim[THREE_DIMENSIONS] = {0, 0, 0};
	unsigned int i;
	float tmpDim;

	findLargestDimInXorY();
    findLargestDimInZ();
    mLargestSingleDim = *std::max_element(mLargestDimInVolumes,
        mLargestDimInVolumes + THREE_DIMENSIONS);
}

void loadedVolumes::findLargestDimInXorY(void)
{
	unsigned int i;
    for (auto& it : mXYresolution)
    {
        for (i = 0; i < TWO_DIMENSIONS; ++i)
    	{
            if( it[i] > mLargestDimInVolumes[i] )
                mLargestDimInVolumes[i] = it[i];
    	}
    }
}

void loadedVolumes::findLargestDimInZ(void)
{
	unsigned int i;
	float tmpDim;
    for (i = 0; i < mNumVolumes; ++i)
    {
    	tmpDim = mSliceThickness[i] * mZslices[i];
    	if( tmpDim > mLargestDimInVolumes[Z_AXIS] )
    		mLargestDimInVolumes[Z_AXIS] = tmpDim;
    }
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
