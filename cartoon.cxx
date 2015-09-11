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
#include <vector>

vtkStandardNewMacro(MouseInteractorStyle2);

std::string createDescription(int boxNum, int size[], double origin[])
{
  char tmpString[50];

  std::string description = "Volume ";
  sprintf(tmpString, "%2d", boxNum);
  description.append(tmpString);
  description += " of size (";
  sprintf(tmpString, "%d,%d,%d", size[0], size[1], size[2]);
  description.append(tmpString);
  description += ") at (";
  sprintf(tmpString, "%d,%d,%d", (int)origin[0], (int)origin[1],
          (int)origin[2]);
  description.append(tmpString);
  description += ")";

  return description;
}

///////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[])
{
/*  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " path/to/data" << endl;
    return EXIT_FAILURE;
  }*/

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

  // Set a background color for the renderer and set the size of the
  // render window (expressed in pixels).
  aRenderer->SetBackground(.2, .3, .4);
  renWin->SetSize(900, 600);

  //Use vtkVolume16Reader object but no image data will be contained for this
  // demo; just create it and set data dimensions for size of the outline
  int numBoxes = 4;
  int boxSize[4][3] = { {1500, 1000, 500},
                        {40,   1200, 1000},
                        {200,   150, 100},
                        { 40,    40,  40} };
  double boxOrigin[4][3] = { {0, 0, 0},
                             {100, 200, 100},
                             {550, 150, 50},
                             {1200, 800, 400} };
  std::vector<std::string> boxDescription(4);
  std::vector<vtkActor*> boxActorPointer(4);

  std::vector <vtkSmartPointer<vtkVolume16Reader> > box(numBoxes);
  std::vector <vtkSmartPointer<vtkOutlineFilter> > outlineData(numBoxes);
  std::vector <vtkSmartPointer<vtkPolyDataMapper> > mapOutline(numBoxes);
  std::vector <vtkSmartPointer<vtkActor> > outline(numBoxes);

  for (int i = 0; i < numBoxes; ++i)
  {
    box[i] = vtkSmartPointer<vtkVolume16Reader>::New();
    box[i]->SetDataDimensions(boxSize[i][0],boxSize[i][1]);
    box[i]->SetImageRange(1, boxSize[i][2]);
    box[i]->SetDataOrigin(boxOrigin[i]);
    box[i]->Update();

    boxDescription[i] = createDescription(i, boxSize[i], boxOrigin[i]);

    outlineData[i] = vtkSmartPointer<vtkOutlineFilter>::New();
    outlineData[i]->SetInputConnection(box[i]->GetOutputPort());
    outlineData[i]->Update();
  
    mapOutline[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapOutline[i]->SetInputConnection(outlineData[i]->GetOutputPort());
  
    outline[i] = vtkSmartPointer<vtkActor>::New();
    outline[i]->SetMapper(mapOutline[i]);
    outline[i]->GetProperty()->SetColor(0,0,0);
    boxActorPointer[i] = outline[i];
    std::cout << "Created outline " << i << " with address: " << outline[i] << std::endl;
  }

  // It is convenient to create an initial view of the data. The
  // FocalPoint and Position form a vector direction. Later on
  // (ResetCamera() method) this vector is used to position the camera
  // to look at the data in this direction.
  vtkSmartPointer<vtkCamera> aCamera =
    vtkSmartPointer<vtkCamera>::New();
  aCamera->SetViewUp (0, 0, -1);
  aCamera->SetPosition (0, 1, 0);
  aCamera->SetFocalPoint (0, 0, 0);
  aCamera->ComputeViewPlaneNormal();
  aCamera->Azimuth(30.0);
  aCamera->Elevation(30.0);

  vtkSmartPointer<MouseInteractorStyle2> style2 =
    vtkSmartPointer<MouseInteractorStyle2>::New();
  style2->SetDefaultRenderer(aRenderer);
  style2->setObjectDescriptions(boxDescription);
  style2->setObjectPointerValues(boxActorPointer);
  iren->SetInteractorStyle(style2);

  //Create hover widget
  vtkSmartPointer<vtkHoverWidget> hoverWidget =
    vtkSmartPointer<vtkHoverWidget>::New();
  hoverWidget->SetInteractor(iren);
  hoverWidget->SetTimerDuration(250);
 
  // Create a callback to listen to the widget's two VTK events
  vtkSmartPointer<vtkHoverCallback> hoverCallback =
    vtkSmartPointer<vtkHoverCallback>::New();
  hoverWidget->AddObserver(vtkCommand::TimerEvent,hoverCallback);
  hoverWidget->AddObserver(vtkCommand::EndInteractionEvent,hoverCallback);
  hoverCallback->setRenderWindowInteractor(iren);
  hoverCallback->setRenderer(aRenderer);
  hoverCallback->setWindowRenderer(renWin);
  hoverCallback->setObjectDescriptions(boxDescription);
  hoverCallback->setObjectPointerValues(boxActorPointer);

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

  return EXIT_SUCCESS;
}
