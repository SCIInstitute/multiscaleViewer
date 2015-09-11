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
#include <vtkStripper.h>
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
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkAnimationCue.h>
#include <vtkAnimationScene.h>
#include <vtkCubeSource.h>
#include <vtkProgrammableFilter.h>
#include <vtkCommand.h>
#include <memory>
#include <vtkCoordinate.h>
#include <vtkRendererCollection.h>
#include <string>

#include <vtkPropPicker.h>
#include <vtkSphereSource.h>

 
class MouseInteractorStyle3 : public vtkInteractorStyleTrackballCamera
{
  public:
    static MouseInteractorStyle3* New();
 
    virtual void OnLeftButtonDown() 
    {
      std::cout << "Left click @ ";
      int x = this->Interactor->GetEventPosition()[0];
      int y = this->Interactor->GetEventPosition()[1];
      std::cout << "(" << x << "," << y << ")" << std::endl;
      vtkSmartPointer<vtkCoordinate> coordinate = 
        vtkSmartPointer<vtkCoordinate>::New();
      coordinate->SetCoordinateSystemToDisplay();
      coordinate->SetValue(x,y,0);
/* 
      // This doesn't produce the right value if the sphere is zoomed in???
      double* world = coordinate->GetComputedWorldValue(this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
      std::cout << "World coordinate: " << world[0] << ", " << world[1] << ", " << world[2] << std::endl;
*/ 
      // Forward events
      vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
};
vtkStandardNewMacro(MouseInteractorStyle3);



class MouseInteractorStyle2 : public vtkInteractorStyleTrackballCamera
{
  public:
    static MouseInteractorStyle2* New();
    vtkTypeMacro(MouseInteractorStyle2, vtkInteractorStyleTrackballCamera);
 
    virtual void OnLeftButtonDown()
    {
      int* clickPos = this->GetInteractor()->GetEventPosition();
 
      // Pick from this location.
      vtkSmartPointer<vtkPropPicker>  picker =
        vtkSmartPointer<vtkPropPicker>::New();
      picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());
 
      double* pos = picker->GetPickPosition();
      std::cout << "Pick position (world coordinates) is: "
                << pos[0] << " " << pos[1]
                << " " << pos[2] << std::endl;
 
      std::cout << "Picked actor: " << picker->GetActor() << std::endl;
      //Create a sphere
      vtkSmartPointer<vtkSphereSource> sphereSource =
        vtkSmartPointer<vtkSphereSource>::New();
      sphereSource->SetCenter(pos[0], pos[1], pos[2]);
      sphereSource->SetRadius(0.1);
 
 
      //this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetDefaultRenderer()->AddActor(actor);
//      this->GetDefaultRenderer()->AddActor(actor);
      // Forward events
      vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
 
  private:
 
};
vtkStandardNewMacro(MouseInteractorStyle2);



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

  //Box 1
  vtkSmartPointer<vtkVolume16Reader> box1;
  box1 = vtkSmartPointer<vtkVolume16Reader>::New();
  box1->SetDataDimensions(1536,1024);
  box1->SetImageRange(1, 500);
  box1->Update();

  // An outline provides context around the data.
  vtkSmartPointer<vtkOutlineFilter> outlineData =
    vtkSmartPointer<vtkOutlineFilter>::New();
  outlineData->SetInputConnection(box1->GetOutputPort());
  outlineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> mapOutline =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapOutline->SetInputConnection(outlineData->GetOutputPort());

  vtkSmartPointer<vtkActor> outline =
    vtkSmartPointer<vtkActor>::New();
  outline->SetMapper(mapOutline);
  outline->GetProperty()->SetColor(0,0,0);

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
  iren->SetInteractorStyle(style2);

  // Actors are added to the renderer.
  aRenderer->AddActor(outline);

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

  iren->Start();

  return EXIT_SUCCESS;
}
