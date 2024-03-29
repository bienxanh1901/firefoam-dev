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

#include "odeNew.H"
#include "ODEChemistryModelNew.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class ODEChemistryType>
Foam::odeNew<ODEChemistryType>::odeNew
(
    const fvMesh& mesh,
    const word& ODEmodeNewlName,
    const word& thermoType
)
:
    chemistrySolver<ODEChemistryType>(mesh, ODEmodeNewlName, thermoType),
    coeffsDict_(this->subDict(ODEmodeNewlName + "Coeffs")),
    solverName_(coeffsDict_.lookup("ODESolver")),
    odeNewSolver_(ODESolver::New(solverName_, *this)),
    eps_(readScalar(coeffsDict_.lookup("eps")))
{}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class ODEChemistryType>
Foam::odeNew<ODEChemistryType>::~odeNew()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class ODEChemistryType>
Foam::scalar Foam::odeNew<ODEChemistryType>::solve
(
    scalarField& c,
    const scalar T,
    const scalar p,
    const scalar t0,
    const scalar dt
) const
{
    label nSpecie = this->nSpecie();
    scalarField c1(this->nEqns(), 0.0);

    // copy the concentration, T and P to the total solve-vector
    for (label i = 0; i < nSpecie; i++)
    {
        c1[i] = c[i];
    }
    c1[nSpecie] = T;
    c1[nSpecie+1] = p;

    scalar dtEst = dt;

    odeNewSolver_->solve
    (
        *this,
        t0,
        t0 + dt,
        c1,
        eps_,
        dtEst
    );

    for (label i=0; i<c.size(); i++)
    {
        c[i] = max(0.0, c1[i]);
    }

    return dtEst;
}


// ************************************************************************* //
