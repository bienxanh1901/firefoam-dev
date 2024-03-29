/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2010-2010 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "regionModel.H"
#include "fvMesh.H"
#include "Time.H"
#include "directMappedWallPolyPatch.H"
#include "zeroGradientFvPatchFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
    defineTypeNameAndDebug(regionModel, 0);
}
}

// * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * //

void Foam::regionModels::regionModel::constructMeshObjects()
{
    // construct region mesh
    regionMeshPtr_.reset
    (
        new fvMesh
        (
            IOobject
            (
                lookup("regionName"),
                time_.timeName(),
                time_,
                IOobject::MUST_READ
            )
        )
    );
}


void Foam::regionModels::regionModel::initialise()
{
    if (debug)
    {
        Pout<< "regionModel::initialise()" << endl;
    }

    label nBoundaryFaces = 0;
    DynamicList<label> primaryPatchIDs;
    DynamicList<label> intCoupledPatchIDs;
    const polyBoundaryMesh& rbm = regionMesh().boundaryMesh();
    const polyBoundaryMesh& pbm = primaryMesh().boundaryMesh();
    mappedPatches_.setSize(rbm.size());

    forAll(rbm, patchI)
    {
        const polyPatch& regionPatch = rbm[patchI];
        if (isA<directMappedWallPolyPatch>(regionPatch))
        {
            if (debug)
            {
                Pout<< "found " << directMappedWallPolyPatch::typeName
                    <<  " " << regionPatch.name() << endl;
            }

            intCoupledPatchIDs.append(patchI);

            nBoundaryFaces += regionPatch.faceCells().size();

            const directMappedWallPolyPatch& dmp =
                refCast<const directMappedWallPolyPatch>(regionPatch);

            const label primaryPatchI = dmp.samplePolyPatch().index();
            primaryPatchIDs.append(primaryPatchI);

            mappedPatches_.set
            (
                patchI,
                new directMappedPatchBase
                (
                    pbm[primaryPatchI],
                    regionMesh().name(),
                    directMappedPatchBase::NEARESTPATCHFACE,
                    regionPatch.name(),
                    vector::zero
                )
            );
        }
    }

    primaryPatchIDs_.transfer(primaryPatchIDs);
    intCoupledPatchIDs_.transfer(intCoupledPatchIDs);
//    mappedPatches_.resize(nCoupledPatches);

    if (nBoundaryFaces == 0)
    {
        WarningIn("regionModel::initialise()")
            << "Region model being applied without direct mapped boundary "
            << "conditions" << endl;
    }
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

bool Foam::regionModels::regionModel::read()
{
    if (regIOobject::read())
    {
        if (active_)
        {
            if (const dictionary* dictPtr = subDictPtr(modelName_ + "Coeffs"))
            {
                coeffs_ <<= *dictPtr;
            }

            infoOutput_.readIfPresent("infoOutput", *this);
        }

        return true;
    }
    else
    {
        return false;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::regionModels::regionModel::regionModel(const fvMesh& mesh)
:
    IOdictionary
    (
        IOobject
        (
            "regionModelProperties",
            mesh.time().constant(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        )
    ),
    primaryMesh_(mesh),
    time_(mesh.time()),
    active_(false),
    infoOutput_(false),
    modelName_("none"),
    regionMeshPtr_(NULL),
    coeffs_(dictionary::null),
    primaryPatchIDs_(),
    intCoupledPatchIDs_(),
    mappedPatches_()
{}


Foam::regionModels::regionModel::regionModel
(
    const fvMesh& mesh,
    const word& regionType,
    const word& modelName,
    bool readFields
)
:
    IOdictionary
    (
        IOobject
        (
            regionType + "Properties",
            mesh.time().constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    ),
    primaryMesh_(mesh),
    time_(mesh.time()),
    active_(lookup("active")),
    infoOutput_(true),
    modelName_(modelName),
    regionMeshPtr_(NULL),
    coeffs_(subOrEmptyDict(modelName + "Coeffs")),
    primaryPatchIDs_(),
    intCoupledPatchIDs_(),
    mappedPatches_()
{
    if (active_)
    {
        constructMeshObjects();
        initialise();

        if (readFields)
        {
            read();
        }
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::regionModels::regionModel::~regionModel()
{}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

void Foam::regionModels::regionModel::preEvolveRegion()
{
    // do nothing
}


void Foam::regionModels::regionModel::evolveRegion()
{
    // do nothing
}

void Foam::regionModels::regionModel::postEvolveRegion()
{
    // do nothing
}


void Foam::regionModels::regionModel::evolve()
{
    if (active_)
    {
        if (primaryMesh_.changing())
        {
            FatalErrorIn("regionModel::evolve()")
                << "Currently not possible to apply " << modelName_
                << " model to moving mesh cases" << nl << abort(FatalError);
        }

        Info<< "\nEvolving " << modelName_ << " for region "
            << regionMesh().name() << endl;

        // Update any input information
        read();

        // Pre-evolve
        preEvolveRegion();

        // Increment the region equations up to the new time level
        evolveRegion();
        
        // Pre-evolve
        postEvolveRegion();

        // Provide some feedback
        if (infoOutput_)
        {
            Info<< incrIndent;
            info();
            Info<< endl << decrIndent;
        }
    }
}


void Foam::regionModels::regionModel::info() const
{
    // do nothing
}


// ************************************************************************* //
