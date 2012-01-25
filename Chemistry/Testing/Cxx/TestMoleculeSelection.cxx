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

#include "vtkActor.h"
#include "vtkCamera.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkExtractSelectedFrustum.h"
#include "vtkGraphToPolyData.h"
#include "vtkHardwareSelector.h"
#include "vtkIdTypeArray.h"
#include "vtkInteractorStyleRubberBandPick.h"
#include "vtkInformation.h"
#include "vtkMolecule.h"
#include "vtkMoleculeMapper.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlanes.h"
#include "vtkPointData.h"
#include "vtkProp3DCollection.h"
#include "vtkRenderedAreaPicker.h"
#include "vtkRenderer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkTrivialProducer.h"
#include "vtkUnstructuredGrid.h"

// Globals for testing
vtkNew<vtkIdTypeArray> atomIds;
vtkNew<vtkIdTypeArray> bondIds;

// Convenience function to print out the atom and bond ids that belong to
// molMap and are contained in sel
void dumpMolSelection(vtkSelection *sel, vtkMoleculeMapper *molMap)
{
  // Extract the atoms and bonds from the selection
  molMap->GetSelectedAtomsAndBonds(sel, atomIds.GetPointer(),
                                   bondIds.GetPointer());

  vtkMolecule *mol = molMap->GetInput();

  // Print selection
  cerr << "\n### Selection ###\n";
  cerr << "Atoms: ";
  for (vtkIdType i = 0; i < atomIds->GetNumberOfTuples(); i++)
    {
    cerr << atomIds->GetValue(i) << " ";
    }
  cerr << "\nBonds: ";
  for (vtkIdType i = 0; i < bondIds->GetNumberOfTuples(); i++)
    {
    vtkBond bond = mol->GetBond(bondIds->GetValue(i));
    cerr << bond.GetId() << " (" << bond.GetBeginAtomId() << "-"
         << bond.GetEndAtomId() << ") ";
    }
  cerr << endl;
}

class MoleculePickCommand : public vtkCommand
{
protected:
  vtkSmartPointer<vtkRenderer> Renderer;
  vtkSmartPointer<vtkAreaPicker> Picker;
  vtkSmartPointer<vtkAlgorithm> MoleculeSource;
  vtkSmartPointer<vtkMoleculeMapper> MoleculeMapper;

public:
  static MoleculePickCommand * New() {return new MoleculePickCommand;}
  vtkTypeMacro(MoleculePickCommand, vtkCommand);

  MoleculePickCommand() :
    Renderer(vtkSmartPointer<vtkRenderer>::New()),
    Picker(vtkSmartPointer<vtkAreaPicker>::New()),
    MoleculeSource(vtkSmartPointer<vtkAlgorithm>::New()),
    MoleculeMapper(vtkSmartPointer<vtkMoleculeMapper>::New()) {}

  virtual ~MoleculePickCommand() {}

  void SetRenderer(vtkRenderer *r) {this->Renderer = r;}
  void SetPicker(vtkAreaPicker *p) {this->Picker = p;}
  void SetMoleculeSource(vtkAlgorithm *m)
    {
    this->MoleculeSource = m;
    }
  void SetMoleculeMapper(vtkMoleculeMapper *m)
    {
    this->MoleculeMapper = m;
    }

  virtual void Execute(vtkObject *, unsigned long, void *)
    {
    vtkProp3DCollection *props = this->Picker->GetProp3Ds();
    if (props->GetNumberOfItems() != 0)
      {
      // If anything was picked during the fast area pick, do a more detailed
      // pick.
      vtkNew<vtkHardwareSelector> selector;
      selector->SetFieldAssociation(vtkDataObject::FIELD_ASSOCIATION_POINTS);
      selector->SetRenderer(this->Renderer);
      selector->SetArea(
            static_cast<unsigned int>(this->Renderer->GetPickX1()),
            static_cast<unsigned int>(this->Renderer->GetPickY1()),
            static_cast<unsigned int>(this->Renderer->GetPickX2()),
            static_cast<unsigned int>(this->Renderer->GetPickY2()));
      // Make the actual pick and pass the result to the convenience function
      // defined earlier
      dumpMolSelection(selector->Select(), this->MoleculeMapper);
      }
    }
};

int TestMoleculeSelection(int argc, char *argv[])
{
  vtkNew<vtkMolecule> mol;

  // Use a trivial producer, since the molecule was created by hand
  vtkNew<vtkTrivialProducer> molSource;
  molSource->SetOutput(mol.GetPointer());

  // Create a 4x4 grid of atoms one angstrom apart
  vtkAtom a1  = mol->AddAtom( 1, 0.0, 0.0, 0.0);
  vtkAtom a2  = mol->AddAtom( 2, 0.0, 1.0, 0.0);
  vtkAtom a3  = mol->AddAtom( 3, 0.0, 2.0, 0.0);
  vtkAtom a4  = mol->AddAtom( 4, 0.0, 3.0, 0.0);
  vtkAtom a5  = mol->AddAtom( 5, 1.0, 0.0, 0.0);
  vtkAtom a6  = mol->AddAtom( 6, 1.0, 1.0, 0.0);
  vtkAtom a7  = mol->AddAtom( 7, 1.0, 2.0, 0.0);
  vtkAtom a8  = mol->AddAtom( 8, 1.0, 3.0, 0.0);
  vtkAtom a9  = mol->AddAtom( 9, 2.0, 0.0, 0.0);
  vtkAtom a10 = mol->AddAtom(10, 2.0, 1.0, 0.0);
  vtkAtom a11 = mol->AddAtom(11, 2.0, 2.0, 0.0);
  vtkAtom a12 = mol->AddAtom(12, 2.0, 3.0, 0.0);
  vtkAtom a13 = mol->AddAtom(13, 3.0, 0.0, 0.0);
  vtkAtom a14 = mol->AddAtom(14, 3.0, 1.0, 0.0);
  vtkAtom a15 = mol->AddAtom(15, 3.0, 2.0, 0.0);
  vtkAtom a16 = mol->AddAtom(16, 3.0, 3.0, 0.0);

  // Add bonds along the grid
  vtkBond b1  = mol->AddBond( a1,  a2, 1);
  vtkBond b2  = mol->AddBond( a2,  a3, 1);
  vtkBond b3  = mol->AddBond( a3,  a4, 1);
  vtkBond b4  = mol->AddBond( a5,  a6, 1);
  vtkBond b5  = mol->AddBond( a6,  a7, 1);
  vtkBond b6  = mol->AddBond( a7,  a8, 1);
  vtkBond b7  = mol->AddBond( a9, a10, 1);
  vtkBond b8  = mol->AddBond(a10, a11, 1);
  vtkBond b9  = mol->AddBond(a11, a12, 1);
  vtkBond b10 = mol->AddBond(a13, a14, 1);
  vtkBond b11 = mol->AddBond(a14, a15, 1);
  vtkBond b12 = mol->AddBond(a15, a16, 1);
  vtkBond b13 = mol->AddBond( a1,  a5, 1);
  vtkBond b14 = mol->AddBond( a2,  a6, 1);
  vtkBond b15 = mol->AddBond( a3,  a7, 1);
  vtkBond b16 = mol->AddBond( a4,  a8, 1);
  vtkBond b17 = mol->AddBond( a5,  a9, 1);
  vtkBond b18 = mol->AddBond( a6, a10, 1);
  vtkBond b19 = mol->AddBond( a7, a11, 1);
  vtkBond b20 = mol->AddBond( a8, a12, 1);
  vtkBond b21 = mol->AddBond( a9, a13, 1);
  vtkBond b22 = mol->AddBond(a10, a14, 1);
  vtkBond b23 = mol->AddBond(a11, a15, 1);
  vtkBond b24 = mol->AddBond(a12, a16, 1);

  // Set up render engine
  vtkNew<vtkMoleculeMapper> molmapper;
  molmapper->SetInput(mol.GetPointer());
  molmapper->UseBallAndStickSettings();
  molmapper->SetAtomicRadiusTypeToUnitRadius();

  vtkNew<vtkActor> actor;
  actor->SetMapper(molmapper.GetPointer());

  vtkNew<vtkRenderer> ren;
  ren->AddActor(actor.GetPointer());
  vtkNew<vtkRenderWindow> win;
  win->AddRenderer(ren.GetPointer());
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(win.GetPointer());

  ren->SetBackground(0.0,0.0,0.0);
  win->SetSize(450,450);
  win->Render();
  // For easier debugging of clipping planes:
  ren->GetActiveCamera()->ParallelProjectionOn();
  ren->GetActiveCamera()->Zoom(2.2);

  // Setup picker
  vtkNew<vtkInteractorStyleRubberBandPick> pickerInt;
  iren->SetInteractorStyle(pickerInt.GetPointer());
  vtkNew<vtkRenderedAreaPicker> picker;
  iren->SetPicker(picker.GetPointer());

  // We'll follow up the cheap RenderedAreaPick with a detailed selection
  // to obtain the atoms and bonds.
  vtkNew<MoleculePickCommand> com;
  com->SetRenderer(ren.GetPointer());
  com->SetPicker(picker.GetPointer());
  com->SetMoleculeSource(molSource.GetPointer());
  com->SetMoleculeMapper(molmapper.GetPointer());
  picker->AddObserver(vtkCommand::EndPickEvent, com.GetPointer());

  // Make pick -- lower left quarter of renderer
  win->Render();
  picker->AreaPick(0, 0, 225, 225, ren.GetPointer());
  win->Render();

  // Interact if desired
  int retVal = vtkRegressionTestImage(win.GetPointer());
  if ( retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    }

  // Verify pick
  if (atomIds->GetValue(0) != 0  ||
      atomIds->GetValue(1) != 1  ||
      atomIds->GetValue(2) != 4  ||
      atomIds->GetValue(3) != 5  ||
      bondIds->GetValue(0) != 0  ||
      bondIds->GetValue(1) != 1  ||
      bondIds->GetValue(2) != 3  ||
      bondIds->GetValue(3) != 4  ||
      bondIds->GetValue(4) != 12 ||
      bondIds->GetValue(5) != 13 ||
      bondIds->GetValue(6) != 16 ||
      bondIds->GetValue(7) != 17 )
    {
    cerr << "Incorrect atoms/bonds picked! (if any picks were performed inter"
            "actively this could be ignored).\n";
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
