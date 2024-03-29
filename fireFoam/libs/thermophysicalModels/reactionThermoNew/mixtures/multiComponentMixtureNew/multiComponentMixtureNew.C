/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2010 OpenCFD Ltd.
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

#include "multiComponentMixtureNew.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

template<class ThermoType>
const ThermoType&
Foam::multiComponentMixtureNew<ThermoType>::constructSpeciesData
(
    const dictionary& thermoDict
)
{
    forAll(species_, i)
    {
        speciesData_.set
        (
            i,
            new ThermoType(thermoDict.lookup(species_[i]))
        );
    }

    return speciesData_[0];
}


template<class ThermoType>
void Foam::multiComponentMixtureNew<ThermoType>::correctMassFractions()
{
    volScalarField Yt = Y_[0];

    for (label n=1; n<Y_.size(); n++)
    {
        Yt += Y_[n];
    }

    forAll (Y_, n)
    {
        Y_[n] /= Yt;
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class ThermoType>
Foam::multiComponentMixtureNew<ThermoType>::multiComponentMixtureNew
(
    const dictionary& thermoDict,
    const wordList& specieNames,
    const HashPtrTable<ThermoType>& specieThermoData,
    const fvMesh& mesh
)
:
    basicMultiComponentMixtureNew(thermoDict, specieNames, mesh),
    speciesData_(species_.size()),
    mixture_("mixture", *specieThermoData[specieNames[0]])
{
    forAll(species_, i)
    {
        speciesData_.set
        (
            i,
            new ThermoType(*specieThermoData[species_[i]])
        );
    }

    correctMassFractions();
}


template<class ThermoType>
Foam::multiComponentMixtureNew<ThermoType>::multiComponentMixtureNew
(
    const dictionary& thermoDict,
    const fvMesh& mesh
)
:
    basicMultiComponentMixtureNew
    (
        thermoDict,
        thermoDict.lookup("species"),
        mesh
    ),
    speciesData_(species_.size()),
    mixture_("mixture", constructSpeciesData(thermoDict))
{
    correctMassFractions();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class ThermoType>
const ThermoType& Foam::multiComponentMixtureNew<ThermoType>::cellMixture
(
    const label celli
) const
{
    mixture_ = Y_[0][celli]/speciesData_[0].W()*speciesData_[0];

    for (label n=1; n<Y_.size(); n++)
    {
        mixture_ += Y_[n][celli]/speciesData_[n].W()*speciesData_[n];
    }

    return mixture_;
}


template<class ThermoType>
const ThermoType& Foam::multiComponentMixtureNew<ThermoType>::patchFaceMixture
(
    const label patchi,
    const label facei
) const
{
    mixture_ =
        Y_[0].boundaryField()[patchi][facei]
       /speciesData_[0].W()*speciesData_[0];

    for (label n=1; n<Y_.size(); n++)
    {
        mixture_ +=
            Y_[n].boundaryField()[patchi][facei]
           /speciesData_[n].W()*speciesData_[n];
    }

    return mixture_;
}


template<class ThermoType>
void Foam::multiComponentMixtureNew<ThermoType>::read
(
    const dictionary& thermoDict
)
{
    forAll(species_, i)
    {
        speciesData_[i] = ThermoType(thermoDict.lookup(species_[i]));
    }
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::nMoles
(
    const label specieI
) const
{
    return speciesData_[specieI].nMoles();
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::W
(
    const label specieI
) const
{
    return speciesData_[specieI].W();
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::Cp
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].Cp(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::Cv
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].Cv(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::H
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].H(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::Hs
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].Hs(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::Hc
(
    const label specieI
) const
{
    return speciesData_[specieI].Hc();
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::S
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].S(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::E
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].E(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::G
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].G(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::A
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].A(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::mu
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].mu(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::kappa
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].kappa(T);
}


template<class ThermoType>
Foam::scalar Foam::multiComponentMixtureNew<ThermoType>::alpha
(
    const label specieI,
    const scalar T
) const
{
    return speciesData_[specieI].alpha(T);
}


// ************************************************************************* //
