/*=========================================================================
  Cartoon visualization of all volumes represented in an input file.
  Usage:
    ./cartoon [path_offset]
  Program will read a legend file in the same directory that contains details
  of all the data volumes to be visualized (default name is load_volumes.txt).
  See the loadedVolumes.h file for specifics on the format of this legend file.
  The optional path_offset will be applied to all paths contained in the
  legend file.
=========================================================================*/
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolume16Reader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkOutlineFilter.h>
#include <vtkCamera.h>
#include <vtkLookupTable.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkProperty.h>
#include <vtkPolyDataNormals.h>
#include <vtkContourFilter.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkOutlineSource.h>
#include <vtkImageActor.h>
#include <vtkSmartPointer.h>
#include <vtkImageMapper3D.h>
#include <vtkHoverWidget.h>
#include <vtkProgrammableFilter.h>
#include <vtkCommand.h>
#include <memory>
#include <vtkCoordinate.h>
#include <vtkRendererCollection.h>
#include <vtkPropPicker.h>
#include <string>
#include "mouse_interact.hpp"
#include "loadedVolumes.hpp"
#include "seg3dHandler.hpp"


int sendToSeg3D_openVolumeCommand(std::string filename);
int sendToSeg3D_openFileSeriesCommand(std::vector<std::string> filenames);

vtkStandardNewMacro(MouseInteractorStyle2);

std::string createDescription(int boxNum, std::array<float, 2> xySize,
                              float zSlices, float sliceThickness,
                              std::array<float, 3> origin)
{
  char tmpString[100];

  std::string description = "Volume ";
  sprintf(tmpString, "%2d", boxNum);
  description.append(tmpString);
  description += " of size (";
  sprintf(tmpString, "%5.1f,%5.1f,%5.1f", (float)xySize[0], (float)xySize[1], (zSlices * sliceThickness));
  description.append(tmpString);
  description += ") at (";
  sprintf(tmpString, "%5.1f,%5.1f,%5.1f", origin[0], origin[1], origin[2]);
  description.append(tmpString);
  description += ")";

  return description;
}

void setBackground(vtkRenderer* render,
                   vtkRenderWindow* renWin)
{
  // Set a background color for the renderer and set the size of the
  // render window (expressed in pixels).
  render->SetBackground(.75, .75, .75);
  renWin->SetSize(900, 600);
}
/*
  void setCubeDimensions(vtkCubeSource* box, const std::array<float, 2> dim2D,
		                 const float z)
  {
    box->SetXLength(dim2D[0]);
    box->SetYLength(dim2D[1]);
    box->SetZLength(z);
  }

  std::array<float, 2> setCubeCenter(vtkCubeSource* box,
                                     std::array<float, 3>& tmpOrigin,
		                             const std::array<float, 2>& dim2D,
		                             const float z)
  {
    //Divide by 2 in order to set cube center rather than origin
    tmpOrigin[0] += dim2D[0] / 2;
    tmpOrigin[1] += dim2D[1] / 2;
    tmpOrigin[2] += z / 2;
    box->SetCenter(tmpOrigin[0], tmpOrigin[1], tmpOrigin[2]);
  }
*/
void setDetailsForEachVolumeAsSpecifiedInFile(vtkOutlineSource* box,
                                              loadedVolumes* volData,
                                             size_t i, std::string& description)
{
  std::array<float, 2> tmpResolution = volData->getXYresolution(i);
  float zDim = volData->getZslices(i) * volData->getSliceThickness(i);
//  setCubeDimensions(box, tmpResolution, zDim);
  std::array<float, 3> tmpOrigin = volData->getOrigin(i);
//  setCubeCenter(box, tmpOrigin, tmpResolution, zDim);
  box->SetBounds(tmpOrigin[0], tmpOrigin[0] + tmpResolution[0],
                 tmpOrigin[1], tmpOrigin[1] + tmpResolution[1],
                 tmpOrigin[2], tmpOrigin[2] + tmpResolution[2] );
  box->Update();

  description = createDescription(i, volData->getXYresolution(i),
    volData->getZslices(i), volData->getSliceThickness(i),
    volData->getOrigin(i));
}

void setupOutlineFilterForSpecificVolume(vtkOutlineFilter* outlineFilter,
                                         vtkOutlineSource* box)
{
  outlineFilter->SetInputConnection(box->GetOutputPort());
  outlineFilter->Update();
}

void setupPolyDataMapperForSpecificVolume(vtkPolyDataMapper* polyMap,
                                          vtkOutlineFilter* outlineFilter)
{

  polyMap->SetInputConnection(outlineFilter->GetOutputPort());
}

void setupActorForSpecificVolume(vtkActor* actor, vtkPolyDataMapper* polyMap)
{
  actor->SetMapper(polyMap);
  actor->GetProperty()->SetColor(0,0,0);
  actor->GetProperty()->SetLineWidth(1.5);
}

void setupCamera(vtkCamera* cam)
{
  cam->SetViewUp (0, 0, -1);
  cam->SetPosition (0, 1, 0);
  cam->SetFocalPoint (0, 0, 0);
  cam->ComputeViewPlaneNormal();
  cam->Azimuth(30.0);
  cam->Elevation(30.0);
}

//Setup the mouse handlers for both left-clicking on an object to load its
// contents/images [MouseInteractorStyle2], and right-clicking (or hovering
// depending on platform) on an object for info [vtkHoverWidget].
void setupMouseControls(vtkHoverWidget* hoverWidget,
    vtkRenderer* aRenderer,
    std::vector<std::string>& boxDescription,
    std::vector<vtkActor*> boxActorPointer,
    seg3dHandler* seg3dHandle,
    vtkRenderWindow* renWin,
    vtkRenderWindowInteractor* iren,
    vtkHoverCallback* hoverCallback,
    MouseInteractorStyle2* style2)
{
  style2->SetDefaultRenderer(aRenderer);
  style2->setObjectDescriptions(boxDescription);
  style2->setObjectPointerValues(boxActorPointer);
  style2->setRenderer(aRenderer);
  style2->setWindowRenderer(renWin);
  style2->setSeg3dHandler(seg3dHandle);
  iren->SetInteractorStyle(style2);

  hoverWidget->SetInteractor(iren);
  hoverWidget->SetTimerDuration(250);
   
  hoverWidget->AddObserver(vtkCommand::TimerEvent,hoverCallback);
  hoverWidget->AddObserver(vtkCommand::EndInteractionEvent,hoverCallback);
  hoverCallback->setRenderWindowInteractor(iren);
  hoverCallback->setRenderer(aRenderer);
  hoverCallback->setWindowRenderer(renWin);
  hoverCallback->setObjectDescriptions(boxDescription);
  hoverCallback->setObjectPointerValues(boxActorPointer);
}

void doStepsToInitializeViewerBeforeStartingVtk(vtkRenderer* renderer,
                                                vtkRenderWindow* renWin,
                                                vtkCamera* cam)
{
    // An initial camera view is created.  The Dolly() method moves
    // the camera towards the FocalPoint, thereby enlarging the image.
    renderer->SetActiveCamera(cam);
  
    // Calling Render() directly on a vtkRenderer is strictly forbidden.
    // Only calling Render() on the vtkRenderWindow is a valid call.
    renWin->Render();

    renderer->ResetCamera();
    cam->Dolly(1.5);
  
    // Note that when camera movement occurs (as it does in the Dolly()
    // method), the clipping planes often need adjusting. Clipping planes
    // consist of two planes: near and far along the view direction. The
    // near plane clips out objects in front of the plane; the far plane
    // clips out objects behind the plane. This way only what is drawn
    // between the planes is actually rendered.
    renderer->ResetCameraClippingRange();
}

void addAllActorsToRenderer(unsigned int num, vtkRenderer* renderer,
                            std::vector<vtkSmartPointer<vtkActor> >& actor)
{
  // Actors are added to the renderer.
  for (unsigned int i = 0; i < num; ++i)
  {
    renderer->AddActor(actor[i]);
  }
}

int setupAndRunVtkEnvironment(std::string pathOffset)
{
  vtkSmartPointer<vtkRenderer> aRenderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin =
    vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(aRenderer);
  vtkSmartPointer<vtkRenderWindowInteractor> iren =
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  setBackground(aRenderer, renWin);

  //Use vtkVolume16Reader object but no image data will be contained for this
  // demo; just create it and set data dimensions for size of the outline
  size_t numBoxes = 0;

  try {
    loadedVolumes volumeData("load_volumes.txt", pathOffset);
    numBoxes = volumeData.getNumLoadedVolumes();

    seg3dHandler seg3dHandle(&volumeData);

    std::vector<std::string> boxDescription(numBoxes);
    std::vector<std::string> boxFilename(numBoxes);
    std::vector<std::vector<std::string> > boxImageSeriesFilenames(numBoxes);
    std::vector<vtkActor*> boxActorPointer(numBoxes);
    std::vector<vtkSmartPointer<vtkOutlineSource> > box(numBoxes);
    std::vector<vtkSmartPointer<vtkOutlineFilter> > outlineData(numBoxes);
    std::vector<vtkSmartPointer<vtkPolyDataMapper> > mapOutline(numBoxes);
    std::vector<vtkSmartPointer<vtkActor> > outline(numBoxes);
  
    for (size_t i = 0; i < numBoxes; ++i)
    {
      box[i] = vtkSmartPointer<vtkOutlineSource>::New();
      outlineData[i] = vtkSmartPointer<vtkOutlineFilter>::New();
      mapOutline[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
      outline[i] = vtkSmartPointer<vtkActor>::New();

      setDetailsForEachVolumeAsSpecifiedInFile(box[i], &volumeData, i,
                                               boxDescription[i]);
      boxFilename[i] = volumeData.getVolFilenames(i);
      boxImageSeriesFilenames[i] = volumeData.getImageSeriesListing(i);
      setupOutlineFilterForSpecificVolume(outlineData[i], box[i]);
      setupPolyDataMapperForSpecificVolume(mapOutline[i], outlineData[i]);
      setupActorForSpecificVolume(outline[i], mapOutline[i]);
      boxActorPointer[i] = outline[i];
      std::cout << "Created outline " << i << " with address: " << outline[i];
      std::cout << std::endl;
    }
    vtkSmartPointer<vtkCamera> aCamera =
      vtkSmartPointer<vtkCamera>::New();
    setupCamera(aCamera);

    //Configure mouse interactions 
    vtkSmartPointer<vtkHoverWidget> hoverWidget =
      vtkSmartPointer<vtkHoverWidget>::New();
    vtkSmartPointer<MouseInteractorStyle2> style2 =
      vtkSmartPointer<MouseInteractorStyle2>::New();
    // Create a callback to listen to the widget's two VTK events
    vtkSmartPointer<vtkHoverCallback> hoverCallback =
      vtkSmartPointer<vtkHoverCallback>::New();
    setupMouseControls(hoverWidget, aRenderer, boxDescription, boxActorPointer,
    		           &seg3dHandle, renWin, iren, hoverCallback, style2);

    addAllActorsToRenderer(numBoxes, aRenderer, outline); 
    doStepsToInitializeViewerBeforeStartingVtk(aRenderer, renWin, aCamera);
  
    // interact with data
    iren->Initialize();
    hoverWidget->On();
    iren->Start();
  } catch (std::string& e) {
    std::cerr << e << std::endl;
    return -1;
  }
  return 0;
}



///////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[])
{
	std::string pathOffset;
	if (argc == 2)
		pathOffset = argv[1];

    setupAndRunVtkEnvironment(pathOffset);

    return 0;
}
