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


//ENABLE_MOUSE_CLICK_SELECTION_OF_ITEMS will highlight a volume bounding box
// if it is clicked-on, and will send box's description to stdout.
#define ENABLE_MOUSE_CLICK_SELECTION_OF_ITEMS

class MouseInteractorStyle2 : public vtkInteractorStyleTrackballCamera
{
  public:
    static MouseInteractorStyle2* New();
    vtkTypeMacro(MouseInteractorStyle2, vtkInteractorStyleTrackballCamera);
    virtual void OnLeftButtonDown(void);
    virtual void setObjectDescriptions(
        const std::vector<std::string>& descriptions);
    virtual void setObjectPointerValues(std::vector<vtkActor*>& pointerValues);
    virtual void setRenderer(vtkRenderer* aRender);
    virtual void setWindowRenderer(vtkRenderWindow* wRender);
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

    virtual void setRenderWindowInteractor(vtkRenderWindowInteractor* rwi);
    virtual void setRenderer(vtkRenderer* aRender);
    virtual void setWindowRenderer(vtkRenderWindow* wRender);
    virtual void setObjectDescriptions(
        const std::vector<std::string>& descriptions);
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
