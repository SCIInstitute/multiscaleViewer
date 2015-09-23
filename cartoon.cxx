/*=========================================================================
  Multiscale demo 01
  Borrowed elements from VTK example Medical3.cxx
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
#include <vector>
//#include <iostream>
//#include <sstream>
//#include <fstream>
#include <cstring>

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

void setDetailsForEachVolumeAsSpecifiedInFile(vtkVolume16Reader* box,
                                              loadedVolumes* volData,
                                             size_t i, std::string& description)
{
  std::array<float, 2> tmpRes = volData->getXYresolution(i);
  box->SetDataDimensions(tmpRes[0], tmpRes[1]);
  box->SetImageRange(1, volData->getZslices(i)
                        * volData->getSliceThickness(i));
  std::array<float, 3> tmpOrigin = volData->getOrigin(i);
  box->SetDataOrigin(tmpOrigin[0], tmpOrigin[1], tmpOrigin[2]);
  box->Update();

  description = createDescription(i, volData->getXYresolution(i),
    volData->getZslices(i), volData->getSliceThickness(i),
    volData->getOrigin(i));
}

void setupOutlineFilterForSpecificVolume(vtkOutlineFilter* outlineFilter,
                                         vtkVolume16Reader* box)
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

void setupMouseControls(vtkHoverWidget* hoverWidget,
                        vtkRenderer* aRenderer,
                        std::vector<std::string> boxDescription,
                        std::vector<vtkActor*> boxActorPointer,
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

int setupVtkEnvironment(void)
{
  // Create the renderer, the render window, and the interactor. The
  // renderer draws into the render window, the interactor enables
  // mouse- and keyboard-based interaction with the data within the
  // render window.
  //
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
    loadedVolumes volumeData("load_volumes.txt");
    numBoxes = volumeData.getNumLoadedVolumes();

    std::vector<std::string> boxDescription(numBoxes);
    std::vector<vtkActor*> boxActorPointer(numBoxes);
    std::vector<vtkSmartPointer<vtkVolume16Reader> > box(numBoxes);
    std::vector<vtkSmartPointer<vtkOutlineFilter> > outlineData(numBoxes);
    std::vector<vtkSmartPointer<vtkPolyDataMapper> > mapOutline(numBoxes);
    std::vector<vtkSmartPointer<vtkActor> > outline(numBoxes);
  
    for (size_t i = 0; i < numBoxes; ++i)
    {
      box[i] = vtkSmartPointer<vtkVolume16Reader>::New();
      outlineData[i] = vtkSmartPointer<vtkOutlineFilter>::New();
      mapOutline[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
      outline[i] = vtkSmartPointer<vtkActor>::New();

      setDetailsForEachVolumeAsSpecifiedInFile(box[i], &volumeData, i,
                                               boxDescription[i]);
      setupOutlineFilterForSpecificVolume(outlineData[i], box[i]);
      setupPolyDataMapperForSpecificVolume(mapOutline[i], outlineData[i]); 
      setupActorForSpecificVolume(outline[i], mapOutline[i]);
      boxActorPointer[i] = outline[i];
      std::cout << "Created outline " << i << " with address: " << outline[i];
      std::cout << std::endl;
    }
    // It is convenient to create an initial view of the data. The
    // FocalPoint and Position form a vector direction. Later on
    // (ResetCamera() method) this vector is used to position the camera
    // to look at the data in this direction.
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

    setupMouseControls(hoverWidget, aRenderer, boxDescription,
                       boxActorPointer, renWin, iren, hoverCallback, style2);

    // Actors are added to the renderer.
    for (int i = 0; i < numBoxes; ++i)
    {
      aRenderer->AddActor(outline[i]);
    }
  
    // An initial camera view is created.  The Dolly() method moves
    // the camera towards the FocalPoint, thereby enlarging the image.
    aRenderer->SetActiveCamera(aCamera);
  
    // Calling Render() directly on a vtkRenderer is strictly forbidden.
    // Only calling Render() on the vtkRenderWindow is a valid call.
    renWin->Render();

    aRenderer->ResetCamera();
    aCamera->Dolly(1.5);
  
    // Note that when camera movement occurs (as it does in the Dolly()
    // method), the clipping planes often need adjusting. Clipping planes
    // consist of two planes: near and far along the view direction. The
    // near plane clips out objects in front of the plane; the far plane
    // clips out objects behind the plane. This way only what is drawn
    // between the planes is actually rendered.
    aRenderer->ResetCameraClippingRange();
  
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
  setupVtkEnvironment();

  return EXIT_SUCCESS;
}
