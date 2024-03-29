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

Class
    Foam::chemistrySolver

Description
    An abstract base class for solving chemistry

SourceFiles
    chemistrySolver.C

\*---------------------------------------------------------------------------*/

#ifndef chemistrySolver_H
#define chemistrySolver_H

#include "ODEChemistryModelNew.H"
#include "IOdictionary.H"
#include "scalarField.H"

#include "autoPtr.H"
#include "runTimeSelectionTables.H"
#include "addToRunTimeSelectionTable.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{


/*---------------------------------------------------------------------------*\
                      Class chemistrySolver Declaration
\*---------------------------------------------------------------------------*/

template<class ODEChemistryType>
class chemistrySolver
:
    public ODEChemistryType
{
protected:

    // Protected data

        //- Name of the chemistry solver
        const word name_;


public:

        //- Runtime type information
        TypeName("chemistrySolver");


        // Declare runtime constructor selection table
        declareRunTimeSelectionTable
        (
            autoPtr,
            chemistrySolver,
            dictionary,
            (
                const fvMesh& mesh,
                const word& ODEmodelName,
                const word& thermoType
            ),
            (mesh, ODEmodelName, thermoType)
        );


    // Constructors

        //- Construct from components
        chemistrySolver
        (
            const fvMesh& mesh,
            const word& ODEmodelName,
            const word& thermoType
        );


    //- Destructor
    virtual ~chemistrySolver();


    // Member Functions

       // - Update the concentrations and return the chemical time
        virtual scalar solve
        (
            scalarField &c,
            const scalar T,
            const scalar p,
            const scalar t0,
            const scalar dt
        ) const = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define makeChemistrySolver(ODEChem)                                          \
                                                                              \
    defineTemplateTypeNameAndDebugWithName                                    \
    (                                                                         \
        chemistrySolver<ODEChem>,                                             \
        "chemistrySolver<"#ODEChem">",                                        \
        0                                                                     \
    );



#define makeChemistrySolverType(SS, ODEChem, Comp, Thermo)                    \
                                                                              \
    typedef SS<ODEChem<Comp, Thermo> > SS##ODEChem##Comp##Thermo;             \
                                                                              \
    defineTemplateTypeNameAndDebugWithName                                    \
    (                                                                         \
        SS##ODEChem##Comp##Thermo,                                            \
        #SS"<"#ODEChem"<"#Comp","#Thermo">>",                                 \
        0                                                                     \
    );                                                                        \
                                                                              \
    addToRunTimeSelectionTable                                                \
    (                                                                         \
        Comp,                                                                 \
        SS##ODEChem##Comp##Thermo,                                            \
        fvMesh                                                                \
    );


#define makeSolidChemistrySolverType(SS, ODEChem, Comp, SThermo, GThermo)     \
                                                                              \
    typedef SS<ODEChem<Comp, SThermo, GThermo> >                              \
        SS##ODEChem##Comp##SThermo##GThermo;                                  \
                                                                              \
    defineTemplateTypeNameAndDebugWithName                                    \
    (                                                                         \
        SS##ODEChem##Comp##SThermo##GThermo,                                  \
        #SS"<"#ODEChem"<"#Comp","#SThermo","#GThermo">>",                     \
        0                                                                     \
    );                                                                        \
                                                                              \
    addToRunTimeSelectionTable                                                \
    (                                                                         \
        Comp,                                                                 \
        SS##ODEChem##Comp##SThermo##GThermo,                                  \
        fvMesh                                                                \
    );


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "chemistrySolver.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //

