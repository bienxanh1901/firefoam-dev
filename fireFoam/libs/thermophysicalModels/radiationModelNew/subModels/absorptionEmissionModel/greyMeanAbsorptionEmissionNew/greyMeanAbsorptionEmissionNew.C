/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2008-2011 OpenCFD Ltd.
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

\*---------------------------------------------------------------------------*/

#include "greyMeanAbsorptionEmissionNew.H"
#include "addToRunTimeSelectionTable.H"
//#include "unitConversion.H"
#include "zeroGradientFvPatchFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    namespace radiation
    {
        defineTypeNameAndDebug(greyMeanAbsorptionEmissionNew, 0);

        addToRunTimeSelectionTable
        (
            absorptionEmissionModel,
            greyMeanAbsorptionEmissionNew,
            dictionary
        );
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::radiation::greyMeanAbsorptionEmissionNew::greyMeanAbsorptionEmissionNew
(
    const dictionary& dict,
    const fvMesh& mesh
)
:
    absorptionEmissionModel(dict, mesh),
    coeffsDict_((dict.subDict(typeName + "Coeffs"))),
    speciesNames_(0),
    specieIndex_(0),
    lookUpTablePtr_(),
    thermo_
    (
        mesh,
        const_cast<basicThermo&>
        (
            mesh.lookupObject<basicThermo>("thermophysicalProperties")
        )
    ),
    EhrrCoeff_(readScalar(coeffsDict_.lookup("EhrrCoeff"))),
    Yj_(nSpecies_)
{
    label nFunc = 0;
    const dictionary& functionDicts = dict.subDict(typeName + "Coeffs");

    forAllConstIter(dictionary, functionDicts, iter)
    {
        // safety:
        if (!iter().isDict())
        {
            continue;
        }
        const word& key = iter().keyword();
        speciesNames_.insert(key, nFunc);
        const dictionary& dict = iter().dict();
        coeffs_[nFunc].initialise(dict);
        nFunc++;
    }

    if (coeffsDict_.found("lookUpTableFileName"))
    {
        const word name = coeffsDict_.lookup("lookUpTableFileName");
        if (name != "none")
        {
            lookUpTablePtr_.set
            (
                new interpolationLookUpTable<scalar>
                (
                    fileName(coeffsDict_.lookup("lookUpTableFileName")),
                    mesh.time().constant(),
                    mesh
                )
            );

            if (!mesh.foundObject<volScalarField>("ft"))
            {
                FatalErrorIn
                (
                    "Foam::radiation::greyMeanAbsorptionEmissionNew(const"
                    "dictionary& dict, const fvMesh& mesh)"
                )   << "specie ft is not present to use with "
                    << "lookUpTableFileName " << nl
                    << exit(FatalError);
            }
        }
    }

    // Check that all the species on the dictionary are present in the
    // look-up table and save the corresponding indices of the look-up table

    label j = 0;
    forAllConstIter(HashTable<label>, speciesNames_, iter)
    {
        if (!lookUpTablePtr_.empty())
        {
            if (lookUpTablePtr_().found(iter.key()))
            {
                label index = lookUpTablePtr_().findFieldIndex(iter.key());

                Info<< "specie: " << iter.key() << " found on look-up table "
                    << " with index: " << index << endl;

                specieIndex_[iter()] = index;
            }
            else if (mesh.foundObject<volScalarField>(iter.key()))
            {
                volScalarField& Y =
                    const_cast<volScalarField&>
                    (
                        mesh.lookupObject<volScalarField>(iter.key())
                    );
                Yj_.set(j, &Y);
                specieIndex_[iter()] = 0;
                j++;
                Info<< "specie: " << iter.key() << " is being solved" << endl;
            }
            else
            {
                FatalErrorIn
                (
                    "Foam::radiation::greyMeanAbsorptionEmissionNew(const"
                    "dictionary& dict, const fvMesh& mesh)"
                )   << "specie: " << iter.key()
                    << " is neither in look-up table: "
                    << lookUpTablePtr_().tableName()
                    << " nor is being solved" << nl
                    << exit(FatalError);
            }
        }
        else if (mesh.foundObject<volScalarField>(iter.key()))
        {
            volScalarField& Y =
                const_cast<volScalarField&>
                (
                    mesh.lookupObject<volScalarField>(iter.key())
                );

            Yj_.set(j, &Y);
            specieIndex_[iter()] = 0;
            j++;
        }
        else
        {
            FatalErrorIn
            (
                "Foam::radiation::greyMeanAbsorptionEmissionNew(const"
                "dictionary& dict, const fvMesh& mesh)"
            )   << " there is not lookup table and the specie" << nl
                << iter.key() << nl
                << " is not found " << nl
                << exit(FatalError);

        }
    }
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::radiation::greyMeanAbsorptionEmissionNew::~greyMeanAbsorptionEmissionNew()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::tmp<Foam::volScalarField>
Foam::radiation::greyMeanAbsorptionEmissionNew::aCont(const label bandI) const
{
    const volScalarField& T = thermo_.thermo().T();
    const volScalarField& p = thermo_.thermo().p();

    const basicMultiComponentMixtureNew& mixture = thermo_.carrier();

    tmp<volScalarField> ta
    (
        new volScalarField
        (
            IOobject
            (
                "a",
                mesh().time().timeName(),
                mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh(),
            dimensionedScalar("a", dimless/dimLength, 0.0),
            zeroGradientFvPatchVectorField::typeName
        )
    );

    scalarField& a = ta().internalField();

    forAll(a, cellI)
    {
        forAllConstIter(HashTable<label>, speciesNames_, iter)
        {
            label n = iter();
            scalar Xipi = 0.0;
            if (specieIndex_[n] != 0)
            {
                //Specie found in the lookUpTable.
                const volScalarField& ft =
                    mesh_.lookupObject<volScalarField>("ft");

                const List<scalar>& Ynft = lookUpTablePtr_().lookUp(ft[cellI]);
                //moles x pressure [atm]
                Xipi = Ynft[specieIndex_[n]]*9.869231e-6*(p[cellI]); //paToAtm
            }
            else
            {
                scalar invWt = 0.0;
                forAll (mixture.Y(), s)
                {
                    invWt += mixture.Y(s)[cellI]/mixture.W(s);
                }

                label index = mixture.species()[iter.key()];
                scalar Xk = mixture.Y(index)[cellI]/(mixture.W(index)*invWt);

                Xipi = Xk*9.869231e-6*(p[cellI]); //pa To Atm
            }

            const absorptionCoeffs::coeffArray& b = coeffs_[n].coeffs(T[cellI]);

            scalar Ti = T[cellI];
            // negative temperature exponents
            if (coeffs_[n].invTemp())
            {
                Ti = 1.0/T[cellI];
            }
            a[cellI] +=
                Xipi
               *(
                    ((((b[5]*Ti + b[4])*Ti + b[3])*Ti + b[2])*Ti + b[1])*Ti
                  + b[0]
                );
        }
    }
    ta().correctBoundaryConditions();
    return ta;
}


Foam::tmp<Foam::volScalarField>
Foam::radiation::greyMeanAbsorptionEmissionNew::eCont(const label bandI) const
{
    tmp<volScalarField> e
    (
        new volScalarField
        (
            IOobject
            (
                "e",
                mesh().time().timeName(),
                mesh(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh(),
            dimensionedScalar("e", dimless/dimLength, 0.0)
        )
    );

    return e;
}


Foam::tmp<Foam::volScalarField>
Foam::radiation::greyMeanAbsorptionEmissionNew::ECont(const label bandI) const
{
    tmp<volScalarField> E
    (
        new volScalarField
        (
            IOobject
            (
                "E",
                mesh_.time().timeName(),
                mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            mesh_,
            dimensionedScalar("E", dimMass/dimLength/pow3(dimTime), 0.0)
        )
    );

    if (mesh_.foundObject<volScalarField>("dQ"))
    {
        const volScalarField& dQ =
            mesh_.lookupObject<volScalarField>("dQ");
        E().internalField() = EhrrCoeff_*dQ;
    }

    return E;
}


// ************************************************************************* //
