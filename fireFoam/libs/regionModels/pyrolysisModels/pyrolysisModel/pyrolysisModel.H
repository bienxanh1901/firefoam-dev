/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2009-2010 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::pyrolysisModel

Description

SourceFiles
    pyrolysisModelI.H
    pyrolysisModel.C

\*---------------------------------------------------------------------------*/

#ifndef pyrolysisModel_H
#define pyrolysisModel_H

#include "runTimeSelectionTables.H"
#include "volFieldsFwd.H"
#include "solidChemistryModel.H"
#include "basicSolidThermo.H"
#include "regionModel1D.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class fvMesh;
class Time;

namespace regionModels
{
namespace pyrolysisModels
{

/*---------------------------------------------------------------------------*\
                      Class pyrolysisModel Declaration
\*---------------------------------------------------------------------------*/

class pyrolysisModel
:
    public regionModel1D
{
private:

    // Private Member Functions

        //- Construct fields
        void constructMeshObjects();

        //- Disallow default bitwise copy construct
        pyrolysisModel(const pyrolysisModel&);

        //- Disallow default bitwise assignment
        void operator=(const pyrolysisModel&);


protected:

    // Protected Data

        //- Flag to indicate whether pyrolysis region coupled to a film region
        bool filmCoupled_;

        //- Pointer to film thickness field
        autoPtr<volScalarField> filmDeltaPtr_;
        
        //- Pointer to film temperature field
        autoPtr<volScalarField> filmTemperaturePtr_;

        //- Pointer to film wall convective heat flux field
        autoPtr<volScalarField> filmConvPtr_;

        //- Film height below which reactions can occur [m]
        scalar reactionDeltaMin_;


    // Protected Member Functions

        //- Read control parameters from dictionary
        virtual bool read();


public:

    //- Runtime type information
    TypeName("pyrolysisModel");


    // Declare runtime constructor selection table

         declareRunTimeSelectionTable
         (
             autoPtr,
             pyrolysisModel,
             mesh,
             (
                const word& modelType,
                const fvMesh& mesh
             ),
             (modelType, mesh)
         );

    // Constructors

        //- Construct null from mesh
        pyrolysisModel(const fvMesh& mesh);

        //- Construct from type name and mesh
        pyrolysisModel(const word& modelType, const fvMesh& mesh);


    // Selectors

        //- Return a reference to the selected surface film model
        static autoPtr<pyrolysisModel> New(const fvMesh& mesh);


    //- Destructor
    virtual ~pyrolysisModel();


    // Member Functions

        // Access

            // Fields

                //- Return density [kg/m3]
                virtual const volScalarField& rho() const = 0;

                //- Return const temperature [K]
                virtual const volScalarField& T() const = 0;

                //- Return specific heat capacity [J/kg/K]
                virtual const tmp<volScalarField> Cp() const = 0;

                //- Return the region absorptivity [1/m]
                virtual const volScalarField& kappa() const = 0;

                //- Return the region thermal conductivity [W/m/k]
                virtual const volScalarField& K() const = 0;

                //- Return the total gas mass flux to primary region [kg/m2/s]
                virtual const surfaceScalarField& phiGas() const = 0;

            // Sources

                //- External hook to add mass to the primary region
                virtual scalar addMassSources
                (
                    const label patchI,
                    const label faceI
                );


        // Evolution

            //- Pre-evolve region
            virtual void preEvolveRegion();


        // Helper function

            //- Mean diffusion number of the solid region
            virtual scalar solidRegionDiffNo() const;

            //- Return max diffusivity allowed in the solid
            virtual scalar maxDiff() const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace pyrolysisModels
} // End namespace regionModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
