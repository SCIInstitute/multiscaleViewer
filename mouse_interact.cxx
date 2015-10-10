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
#include <vtkProperty.h>
#include "mouse_interact.hpp"



static void defineSearchThresholdBoxForPickedObjectNearCursor(
    const int x, const int y, int& xMin, int& xMax,
    int& yMin, int& yMax, int* winSize)
{
    const int thresholdSize_pixel = 2;
  
    xMin = x - thresholdSize_pixel;
    if( xMin < 0 )  xMin = 0;
    xMax = x + thresholdSize_pixel;
    if( xMax > winSize[0] )  xMax = winSize[0];
    yMin = y - thresholdSize_pixel;
    if( yMin < 0 )  yMin = 0;
    yMax = y + thresholdSize_pixel;
    if( yMax > winSize[1] )  yMax = winSize[1];
}

static vtkActor* findPickedActorWithinWiderThreshold(
    vtkSmartPointer<vtkPropPicker>& picker, vtkRenderWindow* renWin,
    vtkRenderer* ren, const int x, const int y)
{
    int xMin, xMax, yMin, yMax;
  
    vtkActor* getActorPicked = NULL;
    defineSearchThresholdBoxForPickedObjectNearCursor(x, y, xMin, xMax,
                                                      yMin, yMax,
                                                      renWin->GetSize());
    //Loop through the larger x/y range to see if any objects are "picked"
    // nearby, this way it will pick without the cursor needing to be right
    // on top of the object
    for (int i = xMin; i <= xMax; ++i)
    {
        for (int j = yMin; j <= yMax; ++j)
        {
            picker->Pick(i, j, 0, ren);
            getActorPicked = picker->GetActor();
            if( getActorPicked != NULL)
                break;
        }
    }
    return getActorPicked;
}



////////////////////// MouseInteractorStyle2
void MouseInteractorStyle2::OnLeftButtonDown()
{
    int* clickPos = this->GetInteractor()->GetEventPosition();

#ifdef ENABLE_MOUSE_CLICK_SELECTION_OF_ITEMS 
  // Pick from this location.
    vtkSmartPointer<vtkPropPicker>  picker =
      vtkSmartPointer<vtkPropPicker>::New();
    picker->Pick(clickPos[0], clickPos[1], 0, this->GetDefaultRenderer());

    vtkActor* getActorPicked = findPickedActorWithinWiderThreshold(picker,
        mRenderWindow, mRenderer, clickPos[0], clickPos[1]);
    resetAllBoxColors();
    for (int j = 0; j < mPointerValues.size(); ++j)
    {
      if( getActorPicked == mPointerValues[j] )
      {
        mPointerValues[j]->GetProperty()->SetColor(1,1,1);
        std::cout << "Selected " << mDescriptions[j] << "." << std::endl;
        if( mClickCallback != NULL && mFilename[j].length() != 0 )
        {
          std::cout << "Opening image: " << mFilename[j] << " in Seg3D." << std::endl;
          mClickCallback(mFilename[j]);
        }
      }
    }
#endif //ENABLE_MOUSE_CLICK_SELECTION_OF_ITEMS 
    // Forward events (this forces a re-render)
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

void MouseInteractorStyle2::setObjectDescriptions(
    const std::vector<std::string>& descriptions)
{
    mDescriptions = descriptions;
}

void MouseInteractorStyle2::setRenderer(vtkRenderer* aRender)
{
    mRenderer = aRender;
}

void MouseInteractorStyle2::setWindowRenderer(vtkRenderWindow* wRender)
{
    mRenderWindow = wRender;
}

void MouseInteractorStyle2::setObjectPointerValues(
    std::vector<vtkActor*>& pointerValues)
{
    mPointerValues = pointerValues;
}

void MouseInteractorStyle2::setCallbacksForClickOnObject(
    int(*callbackImg)(std::string),
    int(*callbackSeries)(std::vector<std::string>&)             )
{
    mClickCallbackSingleImage = callbackImg;
    mClickCallbackImageSeries = callbackSeries;
}

void MouseInteractorStyle2::resetAllBoxColors(void)
{
    for (auto it = mPointerValues.begin(); it != mPointerValues.end(); ++it)
    {
        (*it)->GetProperty()->SetColor(0,0,0);
    }
}


////////////////////// vtkHoverCallback
void vtkHoverCallback::Execute(vtkObject*, unsigned long event,
    void *vtkNotUsed(calldata))
{
    switch (event) 
    {
      case vtkCommand::TimerEvent:
        if( mRenderWindowInteractor != NULL )
        {
            int x, y;
            mRenderWindowInteractor->GetMousePosition(&x, &y);
            // Pick from this location.
            vtkSmartPointer<vtkPropPicker>  picker =
              vtkSmartPointer<vtkPropPicker>::New();
            vtkActor* getActorPicked
                = findPickedActorWithinWiderThreshold(picker, mRenderWindow,
                                                      mRenderer, x, y);
            for (int j = 0; j < mPointerValues.size(); ++j)
            {
                if( getActorPicked == mPointerValues[j] )
                {
                    std::cout << "Hovering over " << mDescriptions[j] << ".";
                    std::cout << std::endl;
                    mPointerValues[j]->GetProperty()->SetColor(1,1,1);
                    //force render so color highlight shows
                    mRenderWindowInteractor->Render(); 
                }
            }
        }
        break;
    
      case vtkCommand::EndInteractionEvent:
          resetAllBoxColors();
          mRenderWindowInteractor->Render(); //force render so highlight shows
          break;
    }
}

void vtkHoverCallback::setRenderWindowInteractor(vtkRenderWindowInteractor* rwi)
{
    mRenderWindowInteractor = rwi;
}

void vtkHoverCallback::setRenderer(vtkRenderer* aRender)
{
    mRenderer = aRender;
}

void vtkHoverCallback::setWindowRenderer(vtkRenderWindow* wRender)
{
    mRenderWindow = wRender;
}

void vtkHoverCallback::setObjectDescriptions(
    const std::vector<std::string>& descriptions)
{
    mDescriptions = descriptions;
}

void vtkHoverCallback::setObjectPointerValues(
    std::vector<vtkActor*>& pointerValues)
{
    mPointerValues = pointerValues;
}

void vtkHoverCallback::resetAllBoxColors(void)
{
    for (auto it = mPointerValues.begin(); it != mPointerValues.end(); ++it)
    {
        (*it)->GetProperty()->SetColor(0,0,0);
    }
}
