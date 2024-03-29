/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2009 OpenCFD Ltd.
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

#include "fixedEnthalpyFluxTemperatureFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "surfaceFields.H"
//#include "turbulenceModel.H"
#include "LESModel.H"
#include "IOobjectList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //
Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::
fixedEnthalpyFluxTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchField<scalar>(p, iF),
    phiName_("phi"),
    rhoName_("none"),
    Tinf_(p.size(), 0.0)
{
    refValue() = 295;
    refGrad() = 0.0;
    valueFraction() = 0.0;
}


Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::
fixedEnthalpyFluxTemperatureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    mixedFvPatchField<scalar>(p, iF),
    phiName_(dict.lookupOrDefault<word>("phi", "phi")),
    rhoName_(dict.lookupOrDefault<word>("rho", "none")),
    Tinf_("Tinf", dict, p.size())
{
    refValue() = Tinf_;
    refGrad() = 0.0;
    valueFraction() = 0.0;

    if (dict.found("value"))
    {
        fvPatchField<scalar>::operator=
        (
            Field<scalar>("value", dict, p.size())
        );
    }
    else
    {
        fvPatchField<scalar>::operator=(refValue());
    }
}

Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::
fixedEnthalpyFluxTemperatureFvPatchScalarField
(
    const fixedEnthalpyFluxTemperatureFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    mixedFvPatchField<scalar>(ptf, p, iF, mapper),
    phiName_(ptf.phiName_),
    rhoName_(ptf.rhoName_),
    Tinf_(ptf.Tinf_, mapper)
{}


Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::
fixedEnthalpyFluxTemperatureFvPatchScalarField
(
    const fixedEnthalpyFluxTemperatureFvPatchScalarField& tppsf
)
:
    mixedFvPatchField<scalar>(tppsf),
    phiName_(tppsf.phiName_),
    rhoName_(tppsf.rhoName_),
    Tinf_(tppsf.Tinf_)
{}

Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::
fixedEnthalpyFluxTemperatureFvPatchScalarField
(
    const fixedEnthalpyFluxTemperatureFvPatchScalarField& tppsf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    mixedFvPatchField<scalar>(tppsf, iF),
    phiName_(tppsf.phiName_),
    rhoName_(tppsf.rhoName_),
    Tinf_(tppsf.Tinf_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    scalarField::autoMap(m);
    Tinf_.autoMap(m);
}


void Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::rmap
(
    const fvPatchScalarField& ptf,
    const labelList& addr
)
{
    mixedFvPatchField<scalar>::rmap(ptf, addr);

    const fixedEnthalpyFluxTemperatureFvPatchScalarField& tiptf =
         refCast<const fixedEnthalpyFluxTemperatureFvPatchScalarField>(ptf);

    Tinf_.rmap(tiptf.Tinf_, addr);
}

void Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::updateCoeffs()
{

    if (this->updated())
    {
        return;
    }

    const label patchI = patch().index();

    HashTable<const compressible::turbulenceModel*>
        turbModels(db().lookupClass<compressible::turbulenceModel>());

    if (turbModels.size() > 1)
    {
        FatalErrorIn
        (
            "Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::"
            "updateCoeffs"
        )   << "more than one turbulence model in the database "
            << nl << exit(FatalError);
    }
    const compressible::turbulenceModel* turbulence = *turbModels.begin();

    const fvsPatchField<scalar>& phip =
        patch().lookupPatchField<surfaceScalarField, scalar>(phiName_);

//    const scalarField& alphap =
//        turbulence->alpha().boundaryField()[patchI];

    const scalarField alphap =
        turbulence->alphaEff()().boundaryField()[patchI];

//    const fvPatchField<scalar>& alphap = 
//	patch().lookupPatchField<volScalarField, scalar>("alphaEff");


//    refValue() = Tinf_;
//    refGrad() = 0.0;
    valueFraction() =
//        1.0/(1.0 - alphap*patch().deltaCoeffs()*patch().magSf()/max(mag(phip), SMALL));
        1.0/(1.0 + alphap*patch().deltaCoeffs()*patch().magSf()/max(mag(phip), SMALL));

//Info << valueFraction() << endl;
//Info << refValue() << endl;

    mixedFvPatchField<scalar>::updateCoeffs();
}


void Foam::fixedEnthalpyFluxTemperatureFvPatchScalarField::
write(Ostream& os) const
{
    fvPatchField<scalar>::write(os);
    os.writeKeyword("phi") << phiName_ << token::END_STATEMENT << nl;
    os.writeKeyword("rho") << rhoName_ << token::END_STATEMENT << nl;
    Tinf_.writeEntry("Tinf", os);
    this->writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        fixedEnthalpyFluxTemperatureFvPatchScalarField
    );

}

// ************************************************************************* //
