#ifndef MOUSE_INTERACT
#define MOUSE_INTERACT

#include <vector>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkHoverWidget.h>
#include <vtkProgrammableFilter.h>
#include <vtkCommand.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkPropPicker.h>
#include <vtkSmartPointer.h>
#include "seg3dHandler.hpp"

/*MouseInteractorStyle2 class
 * Gene Payne
 * This class provides a method for clicking on a volume bounding box
 * in order to select it for viewing purposes.
 * Left-clicking on a volume box will invoke the seg3dHandler callback
 * that corresponds to that volume.
*/
class MouseInteractorStyle2 : public vtkInteractorStyleTrackballCamera
{
  public:
    static MouseInteractorStyle2* New();
    vtkTypeMacro(MouseInteractorStyle2, vtkInteractorStyleTrackballCamera);
    virtual void OnLeftButtonDown(void);
    //Call setObjectDescriptions with a vector of strings to provide a
    // description for each volume in the set. This description is sent
    // to stdout when the volume box is selected.
    virtual void setObjectDescriptions(
        const std::vector<std::string>& descriptions);
    //Call setObjectPointerValues with a vector of pointers to vtkActor
    // objects for each volume bounding box. This provides the mouse interactor
    // with a way to know which VTK actor was selected.
    virtual void setObjectPointerValues(std::vector<vtkActor*>& pointerValues);
    //setRenderer and setWindowRenderer give the mouse interactor pointers to
    // the VTK renderer and window renderer so it can determine which box
    // was selected, and get information about that volume box.
    virtual void setRenderer(vtkRenderer* aRender);
    virtual void setWindowRenderer(vtkRenderWindow* wRender);
    //setSeg3dHandler sets pointer to the seg3dHandler object which will call
    // the Seg3D application to open the volume if selected
    virtual void setSeg3dHandler(seg3dHandler* seg3dH)
        { mSeg3dHandle = seg3dH; }
 
  private:
    std::vector<std::string> mDescriptions;
    std::vector<vtkActor*> mPointerValues;
    void resetAllBoxColors(void);

    vtkRenderer* mRenderer;
    vtkRenderWindow* mRenderWindow;
    seg3dHandler* mSeg3dHandle;
};


/*vtkHoverCallback class
 * Gene Payne
 * This class provides a method for hovering the mouse over a volume bounding
 * box in order to get information about it in stdout without selecting it
 * for detailed viewing. Hovering over an object for about 2 seconds will
 * invoke this functionality.
*/
class vtkHoverCallback : public vtkCommand
{
  public:
    static vtkHoverCallback *New()
    {
      return new vtkHoverCallback;
    }
    vtkHoverCallback() {}
 
    virtual void Execute(vtkObject*, unsigned long event,
        void *vtkNotUsed(calldata));
    //setRenderer, setWindowRenderer, and setRenderWindowInteractor give the
    // mouse hover interactor pointers to the VTK objects that provide this
    // handler with information about which volume box is being hovered-over
    virtual void setRenderWindowInteractor(vtkRenderWindowInteractor* rwi);
    virtual void setRenderer(vtkRenderer* aRender);
    virtual void setWindowRenderer(vtkRenderWindow* wRender);
    //Call setObjectDescriptions with a vector of strings to provide a
    // description for each volume in the set. This description is sent
    // to stdout when the volume box is selected.
    virtual void setObjectDescriptions(
        const std::vector<std::string>& descriptions);
    //Call setObjectPointerValues with a vector of pointers to vtkActor
     // objects for each volume bounding box. This provides the mouse interactor
     // with a way to know which VTK actor was selected.
    virtual void setObjectPointerValues(std::vector<vtkActor*>& pointerValues);

  private:
    void resetAllBoxColors(void);

    std::vector<std::string> mDescriptions;
    std::vector<vtkActor*> mPointerValues;
    vtkRenderWindowInteractor* mRenderWindowInteractor;
    vtkRenderer* mRenderer;
    vtkRenderWindow* mRenderWindow;
};



#endif //#ifdef MOUSE_INTERACT
