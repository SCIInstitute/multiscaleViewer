#ifndef __LOADED_VOLUMES__
#define __LOADED_VOLUMES__

#include <string>
#include <vector>
#include <array>

#define MAX_SINGLE_FILENAME_SIZE 500

class loadedVolumes
{
public:
    loadedVolumes(std::string sourceFilename);
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
    virtual void copyFrom(const loadedVolumes& src);

    size_t mNumVolumes;
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
