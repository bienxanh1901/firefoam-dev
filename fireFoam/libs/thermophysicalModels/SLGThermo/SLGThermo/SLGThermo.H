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

Class
    Foam::SLGThermo

Description
    Thermo package for (S)olids (L)iquids and (G)ases
    Takes reference to thermo package, and provides:
    - carrier : components of thermo - access to elemental properties
    - liquids : liquid components - access  to elemental properties
    - solids  : solid components - access  to elemental properties

    If thermo is not a multi-component thermo package, carrier is NULL.
    Similarly, if no liquids or solids are specified, their respective
    pointers will also be NULL.

    Registered to the mesh so that it can be looked-up

SourceFiles
    SLGThermo.C

\*---------------------------------------------------------------------------*/

#ifndef SLGThermo_H
#define SLGThermo_H

#include "MeshObject.H"
#include "basicThermo.H"
#include "basicMultiComponentMixtureNew.H"
#include "liquidMixture.H"
#include "solidMixture.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                         Class SLGThermo Declaration
\*---------------------------------------------------------------------------*/

class SLGThermo
:
    public MeshObject<fvMesh, SLGThermo>
{
    // Private data

        //- Thermo package
        basicThermo& thermo_;

        //- Reference to the multi-component carrier phase thermo
        basicMultiComponentMixtureNew* carrier_;

        //- Additional liquid properties data
        autoPtr<liquidMixture> liquids_;

        //- Additional solid properties data
        autoPtr<solidMixture> solids_;


public:

    //- Runtime type information
    TypeName("SLGThermo");

    // Constructors

        //- Construct from mesh
        SLGThermo(const fvMesh& mesh, basicThermo& thermo);


    //- Destructor
    virtual ~SLGThermo();


    // Member Functions

        // Access

            //- Return reference to the thermo database
            const basicThermo& thermo() const;

            //- Return reference to the gaseous components
            const basicMultiComponentMixtureNew& carrier() const;

            //- Return reference to the global (additional) liquids
            const liquidMixture& liquids() const;

            //- Return reference to the global (additional) solids
            const solidMixture& solids() const;


            // Index retrieval

                //- Index of carrier component
                label carrierId
                (
                    const word& cmptName,
                    bool allowNotFound = false
                ) const;

                //- Index of liquid component
                label liquidId
                (
                    const word& cmptName,
                    bool allowNotFound = false
                ) const;

                //- Index of solid component
                label solidId
                (
                    const word& cmptName,
                    bool allowNotFound = false
                ) const;


        // Checks

            //- Thermo database has multi-component carrier flag
            bool hasMultiComponentCarrier() const;

            //- Thermo database has liquid components flag
            bool hasLiquids() const;

            //- Thermo database has solid components flag
            bool hasSolids() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
