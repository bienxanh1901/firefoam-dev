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

#include "SurfaceFilmModel.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CloudType>
const Foam::dictionary& Foam::SurfaceFilmModel<CloudType>::dict() const
{
    return dict_;
}


template<class CloudType>
const CloudType& Foam::SurfaceFilmModel<CloudType>::owner() const
{
    return owner_;
}


template<class CloudType>
CloudType& Foam::SurfaceFilmModel<CloudType>::owner()
{
    return owner_;
}


template<class CloudType>
const Foam::fvMesh&  Foam::SurfaceFilmModel<CloudType>::mesh() const
{
    return owner_.mesh();
}


template<class CloudType>
const Foam::dimensionedVector& Foam::SurfaceFilmModel<CloudType>::g() const
{
    return g_;
}


template<class CloudType>
const Foam::dictionary& Foam::SurfaceFilmModel<CloudType>::coeffDict() const
{
    return coeffDict_;
}


template<class CloudType>
Foam::label& Foam::SurfaceFilmModel<CloudType>::nParcelsTransferred()
{
    return nParcelsTransferred_;
}


template<class CloudType>
Foam::label Foam::SurfaceFilmModel<CloudType>::nParcelsTransferred() const
{
    return nParcelsTransferred_;
}


template<class CloudType>
Foam::label& Foam::SurfaceFilmModel<CloudType>::nParcelsInjected()
{
    return nParcelsInjected_;
}


template<class CloudType>
Foam::label Foam::SurfaceFilmModel<CloudType>::nParcelsInjected() const
{
    return nParcelsInjected_;
}


// ************************************************************************* //
