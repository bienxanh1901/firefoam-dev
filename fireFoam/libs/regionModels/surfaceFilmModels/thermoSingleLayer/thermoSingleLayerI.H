/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2009-2010 OpenCFD Ltd.
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

#include "thermoSingleLayer.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace regionModels
{
namespace surfaceFilmModels
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

inline const SLGThermo& thermoSingleLayer::thermo() const
{
    return thermo_;
}


inline label thermoSingleLayer::liquidId() const
{
    return liquidId_;
}


inline tmp<scalarField> thermoSingleLayer::hs
(
    const scalarField& T,
    const label patchI
) const
{
    const scalarField& Cp = Cp_.boundaryField()[patchI];
    return Cp*(T - 298.15);
}


inline tmp<volScalarField> thermoSingleLayer::hs
(
    const volScalarField& T
) const
{
    return tmp<volScalarField>
    (
        new volScalarField
        (
            IOobject
            (
                "hs(" + T.name() + ")",
                time().timeName(),
                regionMesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            Cp_*(T - (dimensionedScalar("Tstd", dimTemperature, 298.15))),
            zeroGradientFvPatchScalarField::typeName
        )
    );
}


inline tmp<volScalarField> thermoSingleLayer::T
(
    const volScalarField& hs
) const
{
    return tmp<volScalarField>
    (
        new volScalarField
        (
            IOobject
            (
                "T(" + hs.name() + ")",
                time().timeName(),
                regionMesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            hs/Cp_ + dimensionedScalar("Tstd", dimTemperature, 298.15),
            zeroGradientFvPatchScalarField::typeName
        )
    );
}


inline const volScalarField& thermoSingleLayer::hsSp() const
{
    return hsSp_;
}


inline const volScalarField& thermoSingleLayer::hsSpPrimary() const
{
    return hsSpPrimary_;
}


inline const volScalarField& thermoSingleLayer::TPrimary() const
{
    return TPrimary_;
}


inline const PtrList<volScalarField>& thermoSingleLayer::YPrimary() const
{
    return YPrimary_;
}


inline const heatTransferModel& thermoSingleLayer::htcs() const
{
    return htcs_();
}


inline const heatTransferModel& thermoSingleLayer::htcw() const
{
    return htcw_();
}


inline const phaseChangeModel& thermoSingleLayer::phaseChange() const
{
    return phaseChange_();
}


inline const filmRadiationModel& thermoSingleLayer::radiation() const
{
    return radiation_();
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace surfaceFilmModels
} // End namespace regionModels
} // End namespace Foam

// ************************************************************************* //
