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
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

vtkStandardNewMacro(MouseInteractorStyle2);

struct VolumePacket
{
    std::string filename;
    float origin[3];
    float xyResolution[2];
    float zSlices;
    float sliceThickness;
};

std::string createDescription(int boxNum, float xySize[2], float zSlices,
                              float sliceThickness, float origin[3])
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
  //sprintf(tmpString, "%d,%d,%d", (int)origin[0], (int)origin[1], (int)origin[2]);
  description.append(tmpString);
  description += ")";

  return description;
}

int readVolumesDescriptionFile(std::vector<VolumePacket>& volData)
{
    const std::string filename = "load_volumes.txt";
    std::ifstream file(filename);
    std::string line;
    std::string tmpString;
    int numEntriesInVolumeFile = 0;
    int totalLinesRead = 0;
    VolumePacket tmpVol;

    if( file.good() )
    {
        enum FileReadState {
            getHeader = 0,
            getFile,
            getOrigin,
            getXYresolution,
            getSlices,
            getSliceThickness,
            numLinesPerVolume /*LEAVE THIS AT END*/
        };
        FileReadState fileState = getHeader;

        while(std::getline(file, line))
        {
            std::stringstream linestream(line);

            switch (fileState)
            {
                case getHeader:
                    std::getline(linestream, tmpString, '\n');
                    if( tmpString.compare("VOLUME") != 0 )
                    {
                        std::string msg = "VOLUME header not found on line ";
                        msg += std::to_string(totalLinesRead);
                        throw std::string(msg);
                    }
                    fileState = getFile;
                    break;

                case getFile:
                    std::getline(linestream, tmpString, '\n');
                    tmpVol.filename = tmpString;
                    fileState = getOrigin;
                    break;

                case getOrigin:
                    std::getline(linestream, tmpString, ' ');
                    tmpVol.origin[0] = strtof(tmpString.c_str(), NULL);
                    std::getline(linestream, tmpString, ' ');
                    tmpVol.origin[1] = strtof(tmpString.c_str(), NULL);
                    std::getline(linestream, tmpString, '\n');
                    tmpVol.origin[2] = strtof(tmpString.c_str(), NULL);
                    fileState = getXYresolution;
                    break;

                case getXYresolution:
                    std::getline(linestream, tmpString, ' ');
                    tmpVol.xyResolution[0] = strtof(tmpString.c_str(), NULL);
                    std::getline(linestream, tmpString, '\n');
                    tmpVol.xyResolution[1] = strtof(tmpString.c_str(), NULL);
                    fileState = getSlices;
                    break;

                case getSlices:
                    std::getline(linestream, tmpString, '\n');
                    tmpVol.zSlices = strtof(tmpString.c_str(), NULL);
                    fileState = getSliceThickness;
                    break;

                case getSliceThickness:
                    std::getline(linestream, tmpString, '\n');
                    tmpVol.sliceThickness = strtof(tmpString.c_str(), NULL);
                    fileState = getHeader;
                    volData.push_back(tmpVol);
                    numEntriesInVolumeFile++;
                    break;

                default:
                    break;
            }
            totalLinesRead++;
        }
        if( totalLinesRead%numLinesPerVolume != 0 || totalLinesRead == 0 )
        {
            throw std::string("Error in reading volume file.");
        }
    }
    else
    {
        throw std::string("Cannot open file for approved drives list.");
    }
    return numEntriesInVolumeFile;
}



///////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[])
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

  // Set a background color for the renderer and set the size of the
  // render window (expressed in pixels).
  aRenderer->SetBackground(.75, .75, .75);
  renWin->SetSize(900, 600);

  //Use vtkVolume16Reader object but no image data will be contained for this
  // demo; just create it and set data dimensions for size of the outline
  int numBoxes = 0;
  std::vector<VolumePacket> volumeData;
  try {
    numBoxes = readVolumesDescriptionFile(volumeData);
  } catch (std::string& e) {
    std::cerr << e << std::endl;
    return -1;
  }

  std::vector<std::string> boxDescription(numBoxes);
  std::vector<vtkActor*> boxActorPointer(numBoxes);

  std::vector <vtkSmartPointer<vtkVolume16Reader> > box(numBoxes);
  std::vector <vtkSmartPointer<vtkOutlineFilter> > outlineData(numBoxes);
  std::vector <vtkSmartPointer<vtkPolyDataMapper> > mapOutline(numBoxes);
  std::vector <vtkSmartPointer<vtkActor> > outline(numBoxes);

  for (int i = 0; i < numBoxes; ++i)
  {
    box[i] = vtkSmartPointer<vtkVolume16Reader>::New();
    box[i]->SetDataDimensions(volumeData[i].xyResolution[0],
                              volumeData[i].xyResolution[1]);
    box[i]->SetImageRange(1, volumeData[i].zSlices * volumeData[i].sliceThickness);
    box[i]->SetDataOrigin(volumeData[i].origin[0],
                          volumeData[i].origin[1], volumeData[i].origin[2]);
    box[i]->Update();

    boxDescription[i] = createDescription(i, volumeData[i].xyResolution,
        volumeData[i].zSlices, volumeData[i].sliceThickness,
        volumeData[i].origin);

    outlineData[i] = vtkSmartPointer<vtkOutlineFilter>::New();
    outlineData[i]->SetInputConnection(box[i]->GetOutputPort());
    outlineData[i]->Update();
  
    mapOutline[i] = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapOutline[i]->SetInputConnection(outlineData[i]->GetOutputPort());
  
    outline[i] = vtkSmartPointer<vtkActor>::New();
    outline[i]->SetMapper(mapOutline[i]);
    outline[i]->GetProperty()->SetColor(0,0,0);
    outline[i]->GetProperty()->SetLineWidth(1.5);
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
  style2->setRenderer(aRenderer);
  style2->setWindowRenderer(renWin);
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
