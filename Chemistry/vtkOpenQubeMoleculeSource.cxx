/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOpenQubeMoleculeSource.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

  =========================================================================*/
#include "vtkOpenQubeMoleculeSource.h"

#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkOpenQubeElectronicData.h"
#include "vtkMolecule.h"

#include <vtksys/stl/vector>

#include <openqube/basisset.h>
#include <openqube/basissetloader.h>
#include <openqube/molecule.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkOpenQubeMoleculeSource);

//----------------------------------------------------------------------------
vtkOpenQubeMoleculeSource::vtkOpenQubeMoleculeSource()
  : vtkDataReader(),
    FileName(NULL)
{
}

//----------------------------------------------------------------------------
vtkOpenQubeMoleculeSource::~vtkOpenQubeMoleculeSource()
{
}

//----------------------------------------------------------------------------
vtkMolecule *vtkOpenQubeMoleculeSource::GetOutput()
{
  return vtkMolecule::SafeDownCast(this->GetOutputDataObject(0));;
}

//----------------------------------------------------------------------------
void vtkOpenQubeMoleculeSource::SetOutput(vtkMolecule *output)
{
  this->GetExecutive()->SetOutputData(0, output);
}

//----------------------------------------------------------------------------
int vtkOpenQubeMoleculeSource::RequestData(
  vtkInformation *,
  vtkInformationVector **,
  vtkInformationVector *outputVector)
{
  vtkMolecule *output = vtkMolecule::SafeDownCast
    (vtkDataObject::GetData(outputVector));

  if (!output)
    {
    vtkWarningMacro(<<"vtkOpenQubeMoleculeSource does not have a vtkMolecule "
                  "as output.");
    return 1;
    }

  // Obtain basis set
  OpenQube::BasisSet *basisSet = 0;
  if (this->BasisSet)
    basisSet = this->BasisSet;
  else
    {
    if (!this->FileName)
      {
      vtkWarningMacro(<<"No FileName or OpenQube::BasisSet specified.");
      return 1;
      }
    // Huge padding, better safe than sorry.
    char basisName[strlen(this->FileName) + 256];
    OpenQube::BasisSetLoader::MatchBasisSet(this->FileName, basisName);
    if (!basisName[0])
      {
      vtkErrorMacro(<< "OpenQube cannot find matching basis set file for '"
                    << this->FileName << "'");
      return 1;
      }
    basisSet = OpenQube::BasisSetLoader::LoadBasisSet(basisName);
    vtkDebugMacro(<<"Loaded basis set file: "<< basisName);
    }

  // Populate vtkMolecule
  const OpenQube::Molecule &oqmol = basisSet->moleculeRef();
  this->CopyOQMoleculeToVtkMolecule(&oqmol, output);

  // Add ElectronicData
  vtkNew<vtkOpenQubeElectronicData> oqed;
  oqed->SetBasisSet(basisSet);
  output->SetElectronicData(oqed.GetPointer());

  return 1;
}

//----------------------------------------------------------------------------
int vtkOpenQubeMoleculeSource::FillOutputPortInformation(int,
                                                         vtkInformation *info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMolecule");
  return 1;
}

//----------------------------------------------------------------------------
void vtkOpenQubeMoleculeSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FileName: " << this->FileName << "\n";
}

//----------------------------------------------------------------------------
void vtkOpenQubeMoleculeSource::CopyOQMoleculeToVtkMolecule(
  const OpenQube::Molecule *oqmol, vtkMolecule *mol)
{
  mol->Initialize();
  // Copy atoms
  double pos[3];
  for (size_t i = 0; i < oqmol->numAtoms(); ++i)
    {
    vtkAtom atom = mol->AddAtom();
    oqmol->atomPos(i, pos);
    atom.SetPosition(pos);
    atom.SetAtomicNumber(oqmol->atomAtomicNumber(i));
    }

  // TODO copy bonds (OQ doesn't currently have bonds)
}
