/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2008-2009 OpenCFD Ltd.
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

#include "SinglePhaseMixture.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class CloudType>
void Foam::SinglePhaseMixture<CloudType>::constructIds()
{
    if (this->phaseProps().size() == 0)
    {
        FatalErrorIn
        (
            "void Foam::SinglePhaseMixture<CloudType>::constructIds()"
        )   << "Phase list is empty" << exit(FatalError);
    }
    else if (this->phaseProps().size() > 1)
    {
        FatalErrorIn
        (
            "void Foam::SinglePhaseMixture<CloudType>::constructIds()"
        )   << "Only one phase permitted" << exit(FatalError);
    }

    switch (this->phaseProps()[0].phase())
    {
        case phaseProperties::GAS:
        {
            idGas_ = 0;
            break;
        }
        case phaseProperties::LIQUID:
        {
            idLiquid_ = 0;
            break;
        }
        case phaseProperties::SOLID:
        {
            idSolid_ = 0;
            break;
        }
        default:
        {
            FatalErrorIn
            (
                "void Foam::SinglePhaseMixture<CloudType>::constructIds()"
            )   << "Unknown phase enumeration" << abort(FatalError);
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CloudType>
Foam::SinglePhaseMixture<CloudType>::SinglePhaseMixture
(
    const dictionary& dict,
    CloudType& owner
)
:
    CompositionModel<CloudType>(dict, owner, typeName),
    idGas_(-1),
    idLiquid_(-1),
    idSolid_(-1)
{
    constructIds();
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CloudType>
Foam::SinglePhaseMixture<CloudType>::~SinglePhaseMixture()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
const Foam::scalarField&
Foam::SinglePhaseMixture<CloudType>::YGas0() const
{
    notImplemented
    (
        "const Foam::scalarField& "
        "Foam::SinglePhaseMixture<CloudType>::YGas0() const"
    );
    return this->phaseProps()[0].Y();
}


template<class CloudType>
const Foam::scalarField&
Foam::SinglePhaseMixture<CloudType>::YLiquid0() const
{
    notImplemented
    (
        "const Foam::scalarField& "
        "Foam::SinglePhaseMixture<CloudType>::YLiquid0() const"
    );
    return this->phaseProps()[0].Y();
}


template<class CloudType>
const Foam::scalarField&
Foam::SinglePhaseMixture<CloudType>::YSolid0() const
{
    notImplemented
    (
        "const Foam::scalarField& "
        "Foam::SinglePhaseMixture<CloudType>::YSolid0() const"
    );
    return this->phaseProps()[0].Y();
}


template<class CloudType>
const Foam::scalarField&
Foam::SinglePhaseMixture<CloudType>::YMixture0() const
{
    return this->phaseProps()[0].Y();
}


template<class CloudType>
Foam::label Foam::SinglePhaseMixture<CloudType>::idGas() const
{
    return idGas_;
}


template<class CloudType>
Foam::label Foam::SinglePhaseMixture<CloudType>::idLiquid() const
{
    return idLiquid_;
}


template<class CloudType>
Foam::label Foam::SinglePhaseMixture<CloudType>::idSolid() const
{
    return idSolid_;
}


// ************************************************************************* //

