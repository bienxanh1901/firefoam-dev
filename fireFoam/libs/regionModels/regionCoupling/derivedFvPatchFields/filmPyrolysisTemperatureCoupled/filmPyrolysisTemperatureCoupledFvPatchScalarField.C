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

#include "filmPyrolysisTemperatureCoupledFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "surfaceFields.H"
#include "pyrolysisModel.H"
#include "surfaceFilmModel.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::filmPyrolysisTemperatureCoupledFvPatchScalarField::
filmPyrolysisTemperatureCoupledFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(p, iF),
    phiName_("phi"),
    rhoName_("rho"),
    deltaWet_(1e-6)
{}


Foam::filmPyrolysisTemperatureCoupledFvPatchScalarField::
filmPyrolysisTemperatureCoupledFvPatchScalarField
(
    const filmPyrolysisTemperatureCoupledFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    phiName_(ptf.phiName_),
    rhoName_(ptf.rhoName_),
    deltaWet_(ptf.deltaWet_)
{}


Foam::filmPyrolysisTemperatureCoupledFvPatchScalarField::
filmPyrolysisTemperatureCoupledFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF),
    phiName_(dict.lookupOrDefault<word>("phi", "phi")),
    rhoName_(dict.lookupOrDefault<word>("rho", "rho")),
    deltaWet_(dict.lookupOrDefault<scalar>("deltaWet", 1e-6))
{
    fvPatchScalarField::operator=(scalarField("value", dict, p.size()));
}


Foam::filmPyrolysisTemperatureCoupledFvPatchScalarField::
filmPyrolysisTemperatureCoupledFvPatchScalarField
(
    const filmPyrolysisTemperatureCoupledFvPatchScalarField& fptpsf
)
:
    fixedValueFvPatchScalarField(fptpsf),
    phiName_(fptpsf.phiName_),
    rhoName_(fptpsf.rhoName_),
    deltaWet_(fptpsf.deltaWet_)
{}


Foam::filmPyrolysisTemperatureCoupledFvPatchScalarField::
filmPyrolysisTemperatureCoupledFvPatchScalarField
(
    const filmPyrolysisTemperatureCoupledFvPatchScalarField& fptpsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(fptpsf, iF),
    phiName_(fptpsf.phiName_),
    rhoName_(fptpsf.rhoName_),
    deltaWet_(fptpsf.deltaWet_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::filmPyrolysisTemperatureCoupledFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    typedef regionModels::surfaceFilmModels::surfaceFilmModel filmModelType;
    typedef regionModels::pyrolysisModels::pyrolysisModel pyrModelType;

    bool filmOk =
        db().objectRegistry::foundObject<filmModelType>
        (
            "surfaceFilmProperties"
        );


    bool pyrOk =
        db().objectRegistry::foundObject<pyrModelType>
        (
            "pyrolysisProperties"
        );

    if (!filmOk || !pyrOk)
    {
        // do nothing on construction - film model doesn't exist yet
        return;
    }

    scalarField& Tp = *this;

    const label patchI = patch().index();

    // Retrieve film model
    const filmModelType& filmModel =
        db().lookupObject<filmModelType>("surfaceFilmProperties");

    const label filmPatchI = filmModel.regionPatchID(patchI);

    const mapDistribute& filmMap = filmModel.mappedPatches()[filmPatchI].map();

    scalarField deltaFilm = filmModel.delta().boundaryField()[filmPatchI];
    filmMap.distribute(deltaFilm);

    scalarField TFilm = filmModel.Ts().boundaryField()[filmPatchI];
    filmMap.distribute(TFilm);


    // Retrieve pyrolysis model
    const pyrModelType& pyrModel =
        db().lookupObject<pyrModelType>("pyrolysisProperties");

    const label pyrPatchI = pyrModel.regionPatchID(patchI);

    const mapDistribute& pyrMap = pyrModel.mappedPatches()[pyrPatchI].map();

    scalarField TPyr = pyrModel.T().boundaryField()[pyrPatchI];
    pyrMap.distribute(TPyr);


    forAll(deltaFilm, i)
    {
        if (deltaFilm[i] > deltaWet_)
        {
            // temperature set by film
            Tp[i] = TFilm[i];
        }
        else
        {
            // temperature set by pyrolysis model
            Tp[i] = TPyr[i];
        }
    }

    fixedValueFvPatchScalarField::updateCoeffs();
}


void Foam::filmPyrolysisTemperatureCoupledFvPatchScalarField::write
(
    Ostream& os
) const
{
    fvPatchScalarField::write(os);
    writeEntryIfDifferent<word>(os, "phi", "phi", phiName_);
    writeEntryIfDifferent<word>(os, "rho", "rho", rhoName_);
    os.writeKeyword("deltaWet") << deltaWet_ << token::END_STATEMENT << nl;
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        filmPyrolysisTemperatureCoupledFvPatchScalarField
    );
}


// ************************************************************************* //
