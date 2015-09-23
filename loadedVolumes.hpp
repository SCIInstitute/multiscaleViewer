#ifndef __LOADED_VOLUMES__
#define __LOADED_VOLUMES__

#include <string>
#include <vector>

class loadedVolumes
{
public:
    loadedVolumes(std::string sourceFilename);
    ~loadedVolumes(void);
    const size_t getNumLoadedVolumes(void) const { return mNumVolumes; }
    const std::array<float, 3> getOrigin(size_t idx)
        const { return mOrigin[idx]; }
    const std::array<float, 2> getXYresolution(size_t idx)
        const { return mXYresolution[idx]; }
    const float getZslices(size_t idx) const { return mZslices[idx]; }
    const float getSliceThickness(size_t idx) const { return mSliceThickness[idx]; }

private:
    size_t readVolumesDescriptionFile(void);

    size_t mNumVolumes;
    std::string mFilename;
    std::vector<std::array<float, 3> > mOrigin;
    std::vector<std::array<float, 2> > mXYresolution;
    std::vector<float> mZslices;
    std::vector<float> mSliceThickness;
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
