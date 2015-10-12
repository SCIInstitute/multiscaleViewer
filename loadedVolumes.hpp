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

class loadedVolumes
{
public:
    loadedVolumes(std::string sourceFilename, std::string pathOffset);
    loadedVolumes(const loadedVolumes& src);
    virtual ~loadedVolumes(void);
    loadedVolumes& operator=(const loadedVolumes& rhs);
    const size_t getNumLoadedVolumes(void)
        const { return mNumVolumes; }
    const std::array<float, 3> getOrigin(size_t idx)
        const { return mOrigin[idx]; }
    const std::array<float, 2> getXYresolution(size_t idx)
        const { return mXYresolution[idx]; }
    const float getZslices(size_t idx)
        const { return mZslices[idx]; }
    const float getSliceThickness(size_t idx)
        const { return mSliceThickness[idx]; }
    const std::string getVolFilenames(size_t idx)
        const { return mVolFilename[idx]; }
    const bool isVolumeImageSeries(size_t idx)
        const { return mIsImageSeries[idx]; }
    const size_t getNumImagesInSeries(size_t idx)
        const { return mImageSeriesFilenames[idx].size(); }
    const std::vector<std::string>& getImageSeriesListing(size_t idx)
        const { return mImageSeriesFilenames[idx]; }

private:
    size_t readVolumesDescriptionFile(void);
    std::string removeNewlineAndConvertToString(char* buffer);
    void populateImageSeriesListFromFileFilterString(std::string filter,
        size_t indexIntoListsOfFiles);
    virtual void addOffsetToPath(std::string& filePathInput);
    virtual void copyFrom(const loadedVolumes& src);

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
