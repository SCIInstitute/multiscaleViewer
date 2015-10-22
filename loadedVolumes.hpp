#ifndef __LOADED_VOLUMES__
#define __LOADED_VOLUMES__

/*
 * The loadedVolumes class is used to read a legend file that by default is
 * in the same directory as the app. This is an ASCII text file with one field
 * per line, with 6 fields per volume:
 *   1. Always the text "VOLUME"
 *   2. Filename path. This may be a path to a specific volume .nrrd file, or
 *      an 'ls' filter command that will select a series of files
 *      (example: ../path/to/files/image_series*.jpg)
 *   3. Origin in xyz coordinates (3 floating-point values separated by spaces)
 *   4. Slice xy size (2 floating-point values separated by spaces)
 *   5. Number of xy slices in the z direction to complete the volume
 *   6. Spacing distance between slices in the z direction
 */
#include <string>
#include <vector>
#include <array>

#define MAX_SINGLE_FILENAME_SIZE 500
#define X_AXIS 0
#define Y_AXIS 1
#define Z_AXIS 2
#define TWO_DIMENSIONS 2
#define THREE_DIMENSIONS 3
#define ALL_THREE_AXES 3

/*loadedVolumes class
 * Gene Payne
 * This class stores information about all of the volumes in a set of volumes
 * that is displayed together in a 3D view. This class does not store the
 * actual volume data, but rather general information about each volume
 * including: origin, X & Y dimensions, number of slices in Z dimension, the
 * spacing between slices in the Z dimension, and the filename(s) of the volume.
 *
 * This information is provided in an ASCII text file that has the following
 * format:
 * There are 6 individual lines per volume
 * Line 1: contains the text "VOLUME" only
 * Line 2: contains an ABSOLUTE file path to the file or series of files to be
 *         displayed with this volume. This can be an individual nrrd file, or
 *         a file filter pattern to specify a series of files that will be
 *         opened and stacked in order to create the volume. Two examples are
 *         below, the first for an individual file and the second for a series:
 *         /Users/Joe/path/to/singleVolumeFile.nrrd
 *         /Users/Joe/path/to/ImageSeries_00*.tiff
 * Line 3: Origin of the volume in 3 dimensions. 3 floating-point values are
 *         provided with a space between them.
 * Line 4: X and Y dimensions of a cross section of the volume in the XY plane,
 *         values floating-point with a space between them.
 * Line 5: Number of slices in the Z dimension (single floating point value)
 * Line 6: Spacing between slices in the Z axis (single floating point value)
*/
class loadedVolumes
{
public:
	//Constructor that accepts parameter sourceFilename, which is the name of
	// the file that contains all information about the set of volumes to be
	// displayed. This file must match the convention described above.
	// The pathOffset parameter is optional. If not an empty string, this
	// string's contents will be prepended to each filename that is specified
	// in sourceFilename (this is currently disabled since absolute paths are
	// currently required).
    loadedVolumes(std::string sourceFilename, std::string pathOffset);
    loadedVolumes(const loadedVolumes& src);
    virtual ~loadedVolumes(void);
    loadedVolumes& operator=(const loadedVolumes& rhs);
    //getNumLoadedVolumes provides the number of volumes that were specified
    // in the description file, and that have been loaded and displayed in the
    // multiscale viewer
    const size_t getNumLoadedVolumes(void)
        const { return mNumVolumes; }
    //getOrigin provides a std::array of 3 floating point values to represent
    // the 3D origin of the volume specified by the input parameter idx
    const std::array<float, 3> getOrigin(size_t idx)
        const { return mOrigin[idx]; }
    //getXYresolution provides a std::array of 2 floating point values that
    // represent the X and Y dimensions of the volume in the XY plane,
    // specified by the input parameter idx
    const std::array<float, 2> getXYresolution(size_t idx)
        const { return mXYresolution[idx]; }
    //getZslices provides the number of slices in Z dimension for a volume
    // specified by idx
    const float getZslices(size_t idx)
        const { return mZslices[idx]; }
    //getSliceThickness provides the floating point value of the spacing
    // between slices in the Z dimension
    const float getSliceThickness(size_t idx)
        const { return mSliceThickness[idx]; }
    //getVolFilenames provides the full path to a filename assigned to a
    // particular volume in the series, specified by idx
    const std::string getVolFilenames(size_t idx)
        const { return mVolFilename[idx]; }
    //isVolumeImageSeries returns true if the volume specified by idx is a
    // volume composed of a series of images. If false, this is a volume
    // represented by a single nrrd file.
    const bool isVolumeImageSeries(size_t idx)
        const { return mIsImageSeries[idx]; }
    //getNumLoadedVolumes provides the number of image files that compose a
    // volume given the volume idx
    const size_t getNumImagesInSeries(size_t idx)
        const { return mImageSeriesFilenames[idx].size(); }
    //getImageSeriesListing returns a vector of strings that contain the
    // filename of each individual file that was found to match the
    // file pattern of mVolFilename for index 'idx'. This may be a large
    // number of files.
    const std::vector<std::string>& getImageSeriesListing(size_t idx)
        const { return mImageSeriesFilenames[idx]; }

private:
    size_t readVolumesDescriptionFile(void);
    std::string removeNewlineAndConvertToString(char* buffer);
    void populateImageSeriesListFromFileFilterString(std::string filter,
        size_t indexIntoListsOfFiles);
    virtual void addOffsetToPath(std::string& filePathInput);
    virtual void copyFrom(const loadedVolumes& src);
    void scaleVolumesThatAreTooSmallToBeVisible(void);
    void findLargestSingleDimensionInVolumeSet(void);
    unsigned int checkIfSingleDimIsTooSmall(float dim);
    unsigned int getNumAxesThatAreTooSmall(
   	    float (&tmpDim)[THREE_DIMENSIONS]);
    void findLargestDimInXorY(void);
    void findLargestDimInZ(void);
    void scaleUpAnIndividualVolume(unsigned int volIndex,
        float (&tmpDim)[THREE_DIMENSIONS]);
    void getThreeDimsOfThisVolume(unsigned int volIndex,
    	                          float (&tmpDim)[THREE_DIMENSIONS]);

    size_t mNumVolumes;
    std::string mPathOffset;
    std::string mSourceFilename;
    std::vector<std::string> mVolFilename;
    std::vector<std::vector<std::string> > mImageSeriesFilenames;
    std::vector<std::array<float, 3> > mOrigin;
    std::vector<std::array<float, 2> > mXYresolution;
    std::vector<float> mZslices;
    std::vector<float> mSliceThickness;
    std::vector<bool> mIsImageSeries;
    float mLargestDimInVolumes[3] = {0, 0, 0};
    float mLargestSingleDim = 0;
    const float mSmallestAcceptableSizeRatio = 0.04;

    enum FileReadState {
        parseHeader = 0,
        parseFile,
        parseOrigin,
        parseXYresolution,
        parseSlices,
        parseSliceThickness,
        numLinesPerVolume /*LEAVE THIS AT END*/
    };
};

#endif //__LOADED_VOLUMES__
