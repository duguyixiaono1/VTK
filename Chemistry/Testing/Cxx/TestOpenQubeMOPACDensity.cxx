/*=========================================================================

  Program:   Visualization Toolkit

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"

#include "vtkAbstractElectronicData.h"
#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkColorTransferFunction.h"
#include "vtkImageData.h"
#include "vtkImageShiftScale.h"
#include "vtkMolecule.h"
#include "vtkMoleculeMapper.h"
#include "vtkNew.h"
#include "vtkOpenQubeMoleculeSource.h"
#include "vtkPiecewiseFunction.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"

#include <openqube/basissetloader.h>
#include <openqube/basisset.h>

int TestOpenQubeMOPACDensity(int argc, char *argv[])
{
  char* fname = vtkTestUtilities::ExpandDataFileName(
    argc, argv, "Data/2h2o.out");

  vtkNew<vtkOpenQubeMoleculeSource> oq;
  oq->SetFileName(fname);
  oq->Update();

  vtkSmartPointer<vtkMolecule> mol = vtkSmartPointer<vtkMolecule>::New();
  mol = oq->GetOutput();

  vtkNew<vtkMoleculeMapper> molMapper;
  molMapper->SetInput(mol);
  molMapper->UseLiqouriceStickSettings();
  molMapper->SetBondRadius(0.1);
  molMapper->SetAtomicRadiusScaleFactor(0.1);

  vtkNew<vtkActor> molActor;
  molActor->SetMapper(molMapper.GetPointer());

  vtkAbstractElectronicData *edata = oq->GetOutput()->GetElectronicData();
  if (!edata)
    {
    cout << "NULL vtkAbstractElectronicData returned from "
            "vtkOpenQubeElectronicData.\n";
    return EXIT_FAILURE;
    }

  cout << "Num electrons: " << edata->GetNumberOfElectrons() << "\n";

  vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
  data = edata->GetElectronDensity();
  if (!data)
    {
    cout << "NULL vtkImageData returned from vtkOpenQubeElectronicData.\n";
    return EXIT_FAILURE;
    }

  double range[2];
  data->Update();
  data->GetScalarRange(range);
  cout << "ImageData range: " << range[0] <<" "<< range[1] << "\n";

  vtkNew<vtkImageShiftScale> t;
  t->SetInput(data.GetPointer());
  t->SetShift(0.0);
  double magnitude = range[1];
  if(fabs(magnitude) < 1e-10)
    magnitude = 1.0;
  t->SetScale(255.0/magnitude);
  t->SetOutputScalarTypeToDouble();

  cout << "magnitude: " << magnitude << "\n";

  t->Update();
  t->GetOutput()->GetScalarRange(range);
  cout << "Shifted min/max: " << range[0] << " " << range[1] << "\n";

  vtkNew<vtkPiecewiseFunction> compositeOpacity;
  compositeOpacity->AddPoint(  0.000, 0.00);
  compositeOpacity->AddPoint(  0.001, 0.00);
  compositeOpacity->AddPoint(  5.000, 0.45);
//  compositeOpacity->AddPoint( 10.000, 0.45);
  compositeOpacity->AddPoint(255.000, 0.90);

  vtkNew<vtkColorTransferFunction> color;
  color->AddRGBPoint(  0.000, 0.0, 0.0, 0.00);
  color->AddRGBPoint(  0.001, 0.0, 0.0, 0.20);
  color->AddRGBPoint(  5.000, 0.0, 0.0, 0.50);
  color->AddRGBPoint(255.000, 0.0, 0.0, 1.00);

  vtkNew<vtkSmartVolumeMapper> volumeMapper;
  volumeMapper->SetInputConnection(t->GetOutputPort());
  volumeMapper->SetBlendModeToComposite();
  volumeMapper->SetInterpolationModeToLinear();

  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->ShadeOff();
  volumeProperty->SetInterpolationTypeToLinear();
  volumeProperty->SetScalarOpacity(compositeOpacity.GetPointer());
  volumeProperty->SetColor(color.GetPointer());

  vtkNew<vtkVolume> volume;
  volume->SetMapper(volumeMapper.GetPointer());
  volume->SetProperty(volumeProperty.GetPointer());

  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> win;
  win->AddRenderer(ren.GetPointer());
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(win.GetPointer());

  ren->AddActor(volume.GetPointer());
  ren->AddActor(molActor.GetPointer());

  ren->SetBackground(0.0, 0.0, 0.0);
  win->SetSize(450,450);
  win->Render();
  ren->GetActiveCamera()->Zoom(2.4);
  win->Render();

  int retVal = vtkRegressionTestImage(win.GetPointer());
  if ( retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    }

  cout << volumeMapper->GetLastUsedRenderMode() << "\n";
  return retVal;
}
