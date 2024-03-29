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

\*---------------------------------------------------------------------------*/

#include "ODESolidChemistryModel.H"
#include "reactingSolidMixture.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class CompType, class SolidThermo, class GasThermo>
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
ODESolidChemistryModel
(
    const fvMesh& mesh,
    const word& compTypeName,
    const word& solidThermoName
)
:
    CompType(mesh, solidThermoName),
    ODE(),
    Ys_(this->solidThermo().composition().Y()),
    pyrolisisGases_
    (
        mesh.lookupObject<dictionary>
            ("chemistryProperties").lookup("species")
    ),
    reactions_
    (
        static_cast<const reactingSolidMixture<SolidThermo>& >
            (this->solidThermo().composition())
    ),
    solidThermo_
    (
        static_cast<const reactingSolidMixture<SolidThermo>& >
            (this->solidThermo().composition()).solidData()
    ),
    gasThermo_(pyrolisisGases_.size()),
    nGases_(pyrolisisGases_.size()),
    nSpecie_(Ys_.size() + nGases_),
    nSolids_(Ys_.size()),
    nReaction_(reactions_.size()),
    RRs_(nSolids_),
    RRg_(nGases_),
    Ys0_(nSolids_),
    cellCounter_(0),
    reactingCells_(mesh.nCells(), true)
{
    // create the fields for the chemistry sources
    forAll(RRs_, fieldI)
    {
        RRs_.set
        (
            fieldI,
            new scalarField(mesh.nCells(), 0.0)
        );


        IOobject header
        (
            Ys_[fieldI].name() + "_0",
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ
        );

        // check if field exists and can be read
        if (header.headerOk())
        {
            Ys0_.set
            (
                fieldI,
                new volScalarField
                (
                    IOobject
                    (
                        Ys_[fieldI].name() + "_0",
                        mesh.time().timeName(),
                        mesh,
                        IOobject::MUST_READ,
                        IOobject::AUTO_WRITE
                    ),
                    mesh
                )
            );
        }
        else
        {
            volScalarField Y0Default
            (
                IOobject
                (
                    "Y0Default",
                    mesh.time().timeName(),
                    mesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                mesh
            );

            Ys0_.set
            (
                fieldI,
                new volScalarField
                (
                    IOobject
                    (
                        Ys_[fieldI].name() + "_0",
                        mesh.time().timeName(),
                        mesh,
                        IOobject::NO_READ,
                        IOobject::AUTO_WRITE
                    ),
                    Y0Default
                )
            );
        }

        // Calculate inital values of Ysi0 = rho*delta*Yi
        Ys0_[fieldI].internalField() =
            //this->solidThermo().rho()*Ys_[fieldI]*mesh.V();
            this->solidThermo().rho()*max(Ys_[fieldI],0.001)*mesh.V();
   }

    forAll(RRg_, fieldI)
    {
        RRg_.set(fieldI, new scalarField(mesh.nCells(), 0.0));
    }

    dictionary thermoDict =
        mesh.lookupObject<dictionary>("chemistryProperties");

    forAll(gasThermo_, gasI)
    {
        gasThermo_.set
        (
            gasI,
            new GasThermo(thermoDict.lookup(pyrolisisGases_[gasI]))
        );
    }

    Info<< "ODESolidChemistryModel: Number of solids = " << nSolids_
        << " and reactions = " << nReaction_ << endl;

    Info<< "Number of gases from pyrolysis = " << nGases_ << endl;

    forAll(reactions_, i)
    {
        Info<< indent << "Reaction " << i << nl << reactions_[i] << nl;
    }

}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class CompType, class SolidThermo, class GasThermo>
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
~ODESolidChemistryModel()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class CompType, class SolidThermo, class GasThermo>
Foam::scalarField Foam::
ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::omega
(
    const scalarField& c,
    const scalar T,
    const scalar p,
    const bool updateC0
) const
{
    scalar pf, cf, pr, cr;
    label lRef, rRef;

    const label cellI = cellCounter_;

    scalarField om(nEqns(), 0.0);

    forAll(reactions_, i)
    {
        const solidReaction& R = reactions_[i];

        scalar omegai = omega
        (
            R, c, T, 0.0, pf, cf, lRef, pr, cr, rRef
        );
        scalar rhoL = 0.0;
        forAll(R.slhs(), s)
        {
            label si = R.slhs()[s];
            om[si] -= omegai;
            rhoL = solidThermo_[si].rho(T);
        }
        scalar sr = 0.0;
        forAll(R.srhs(), s)
        {
            label si = R.srhs()[s];
            scalar rhoR = solidThermo_[si].rho(T);
            sr = rhoR/rhoL;
            om[si] += sr*omegai;

            if (updateC0)
            {
                Ys0_[si][cellI] += sr*omegai;
            }
        }
        forAll(R.grhs(), g)
        {
            label gi = R.grhs()[g];
            om[gi + nSolids_] += (1.0 - sr)*omegai;
        }
    }

    return om;
}


template<class CompType, class SolidThermo, class GasThermo>
Foam::scalar
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::omega
(
    const solidReaction& R,
    const scalarField& c,
    const scalar T,
    const scalar p,
    scalar& pf,
    scalar& cf,
    label& lRef,
    scalar& pr,
    scalar& cr,
    label& rRef
) const
{
    scalarField c1(nSpecie_, 0.0);

    label cellI = cellCounter_;

    for (label i=0; i<nSpecie_; i++)
    {
        c1[i] = max(0.0, c[i]);
    }

    scalar kf = R.kf(T, 0.0, c1);

    scalar exponent = R.nReact();

    const label Nl = R.slhs().size();

    for (label s=0; s<Nl; s++)
    {
        label si = R.slhs()[s];

        kf *=
//            pow(c1[si]/max(Ys0_[si][cellI], 0.001), exponent)
            pow(c1[si]/Ys0_[si][cellI], exponent)
           *(Ys0_[si][cellI]);
    }

    return kf;
}


template<class CompType, class SolidThermo, class GasThermo>
void Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::derivatives
(
    const scalar time,
    const scalarField &c,
    scalarField& dcdt
) const
{
    scalar T = c[nSpecie_];

    dcdt = 0.0;

    dcdt = omega(c, T, 0);

    //Total mass concentration
    scalar cTot = 0.0;
    for (label i=0; i<nSolids_; i++)
    {
        cTot += c[i];
    }

    scalar newCp = 0.0;
    scalar newhi = 0.0;
    for (label i=0; i<nSolids_; i++)
    {
        scalar dYidt = dcdt[i]/cTot;
        scalar Yi = c[i]/cTot;
        newCp += Yi*solidThermo_[i].Cp(T);
        //newhi += dYidt*solidThermo_[i].hf();
        newhi -= dYidt*solidThermo_[i].hf();
    }

    scalar dTdt = newhi/newCp;
    scalar dtMag = min(500.0, mag(dTdt));
    dcdt[nSpecie_] = dTdt*dtMag/(mag(dTdt) + 1.0e-10);

    // dp/dt = ...
    dcdt[nSpecie_ + 1] = 0.0;
}


template<class CompType, class SolidThermo, class GasThermo>
void Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::jacobian
(
    const scalar t,
    const scalarField& c,
    scalarField& dcdt,
    scalarSquareMatrix& dfdc
) const
{
    scalar T = c[nSpecie_];

    scalarField c2(nSpecie_, 0.0);

    for (label i=0; i<nSolids_; i++)
    {
        c2[i] = max(c[i], 0.0);
    }

    for (label i=0; i<nEqns(); i++)
    {
        for (label j=0; j<nEqns(); j++)
        {
            dfdc[i][j] = 0.0;
        }
    }

    // length of the first argument must be nSolids
    dcdt = omega(c2, T, 0.0);

    for (label ri=0; ri<reactions_.size(); ri++)
    {
        const solidReaction& R = reactions_[ri];

        scalar kf0 = R.kf(T, 0.0, c2);

        forAll(R.slhs(), j)
        {
            label sj = R.slhs()[j];
            scalar kf = kf0;
            forAll(R.slhs(), i)
            {
                label si = R.slhs()[i];
                scalar exp = R.nReact();
                if (i == j)
                {
                    if (exp < 1.0)
                    {
                        if (c2[si]>SMALL)
                        {
                            kf *= exp*pow(c2[si] + VSMALL, exp - 1.0);
                        }
                        else
                        {
                            kf = 0.0;
                        }
                    }
                    else
                    {
                        kf *= exp*pow(c2[si], exp - 1.0);
                    }
                }
                else
                {
                    Info<< "Solid reactions have only elements on slhs"
                        << endl;
                    kf = 0.0;
                }
            }

            forAll(R.slhs(), i)
            {
                label si = R.slhs()[i];
                dfdc[si][sj] -= kf;
            }
            forAll(R.srhs(), i)
            {
                label si = R.srhs()[i];
                dfdc[si][sj] += kf;
            }
        }
    }

    // calculate the dcdT elements numerically
    scalar delta = 1.0e-8;
    scalarField dcdT0 = omega(c2, T - delta, 0);
    scalarField dcdT1 = omega(c2, T + delta, 0);

    for (label i=0; i<nEqns(); i++)
    {
        dfdc[i][nSpecie_] = 0.5*(dcdT1[i] - dcdT0[i])/delta;
    }

}


template<class CompType, class SolidThermo, class GasThermo>
Foam::tmp<Foam::volScalarField>
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::tc() const
{
    notImplemented
    (
        "ODESolidChemistryModel::tc()"
    );

    return volScalarField::null();
}


template<class CompType, class SolidThermo, class GasThermo>
Foam::tmp<Foam::volScalarField>
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::Sh() const
{
    tmp<volScalarField> tSh
    (
        new volScalarField
        (
            IOobject
            (
                "Sh",
                this->mesh_.time().timeName(),
                this->mesh_,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE,
                false
            ),
            this->mesh_,
            dimensionedScalar("zero", dimEnergy/dimTime/dimVolume, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    if (this->chemistry_)
    {
        scalarField& Sh = tSh();

        forAll(Ys_, i)
        {
            forAll(Sh, cellI)
            {
                scalar hf = solidThermo_[i].hf();
                //Sh[cellI] += hf*RRs_[i][cellI];
                Sh[cellI] -= hf*RRs_[i][cellI];
            }
        }
    }

    return tSh;
}


template<class CompType, class SolidThermo, class GasThermo>
Foam::tmp<Foam::volScalarField>
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::dQ() const
{
    tmp<volScalarField> tdQ
    (
        new volScalarField
        (
            IOobject
            (
                "dQ",
                this->mesh_.time().timeName(),
                this->mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE,
                false
            ),
            this->mesh_,
            dimensionedScalar("dQ", dimEnergy/dimTime, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    if (this->chemistry_)
    {
        volScalarField& dQ = tdQ();
        dQ.dimensionedInternalField() = this->mesh_.V()*Sh()();
    }

    return tdQ;
}


template<class CompType, class SolidThermo, class GasThermo>
Foam::label Foam::
ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::nEqns() const
{
    // nEqns = number of solids + gases + temperature + pressure
    return (nSpecie_ + 2);
}


template<class CompType, class SolidThermo, class GasThermo>
void Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
calculate()
{

    const volScalarField rho
    (
        IOobject
        (
            "rho",
            this->time().timeName(),
            this->mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        this->solidThermo().rho()
    );

    if (this->mesh().changing())
    {
        forAll(RRs_, i)
        {
            RRs_[i].setSize(rho.size());
        }
        forAll(RRg_, i)
        {
            RRg_[i].setSize(rho.size());
        }
    }

    forAll(RRs_, i)
    {
        RRs_[i] = 0.0;
    }
    forAll(RRg_, i)
    {
        RRg_[i] = 0.0;
    }

    if (this->chemistry_)
    {
        forAll(rho, celli)
        {
            cellCounter_ = celli;

            const scalar delta = this->mesh().V()[celli];

            if (reactingCells_[celli])
            {
                scalar rhoi = rho[celli];
                scalar Ti = this->solidThermo().T()[celli];

                scalarField c(nSpecie_, 0.0);
                for (label i=0; i<nSolids_; i++)
                {
                    c[i] = rhoi*Ys_[i][celli]*delta;
                }

                const scalarField dcdt = omega(c, Ti, 0.0, true);

                forAll(RRs_, i)
                {
                    RRs_[i][celli] = dcdt[i]/delta;
                }

                forAll(RRg_, i)
                {
                    RRg_[i][celli] = dcdt[nSolids_ + i]/delta;
                }
            }
        }
    }
}


template<class CompType, class SolidThermo, class GasThermo>
Foam::scalar
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::solve
(
    const scalar t0,
    const scalar deltaT
)
{
    const volScalarField rho
    (
        IOobject
        (
            "rho",
            this->time().timeName(),
            this->mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        this->solidThermo().rho()
    );

    if (this->mesh().changing())
    {
        forAll(RRs_, i)
        {
            RRs_[i].setSize(rho.size());
        }
        forAll(RRg_, i)
        {
            RRg_[i].setSize(rho.size());
        }
    }

    forAll(RRs_, i)
    {
        RRs_[i] = 0.0;
    }
    forAll(RRg_, i)
    {
        RRg_[i] = 0.0;
    }

    if (!this->chemistry_)
    {
        return GREAT;
    }

    scalar deltaTMin = GREAT;

    forAll(rho, celli)
    {
        if (reactingCells_[celli])
        {
            cellCounter_ = celli;

            scalar rhoi = rho[celli];
            scalar Ti = this->solidThermo().T()[celli];

            scalarField c(nSpecie_, 0.0);
            scalarField c0(nSpecie_, 0.0);
            scalarField dc(nSpecie_, 0.0);

            scalar delta = this->mesh().V()[celli];

            for (label i=0; i<nSolids_; i++)
            {
                c[i] = rhoi*Ys_[i][celli]*delta;
            }

            c0 = c;

            scalar t = t0;
            scalar tauC = this->deltaTChem_[celli];
            scalar dt = min(deltaT, tauC);
            scalar timeLeft = deltaT;

            // calculate the chemical source terms
            while (timeLeft > SMALL)
            {
                tauC = this->solve(c, Ti, 0.0, t, dt);
                t += dt;

                // update the temperature
                scalar cTot = 0.0;

                //Total mass concentration
                for (label i=0; i<nSolids_; i++)
                {
                    cTot += c[i];
                }

                scalar newCp = 0.0;
                scalar newhi = 0.0;
                scalar invRho = 0.0;
                scalarList dcdt = (c - c0)/dt;

                for (label i=0; i<nSolids_; i++)
                {
                    scalar dYi = dcdt[i]/cTot;
                    scalar Yi = c[i]/cTot;
                    newCp += Yi*solidThermo_[i].Cp(Ti);
                    //newhi += dYi*solidThermo_[i].hf();
                    newhi -= dYi*solidThermo_[i].hf();
                    invRho += Yi/solidThermo_[i].rho(Ti);
                }

                scalar dTi = (newhi/newCp)*dt;

                Ti += dTi;

                timeLeft -= dt;
                this->deltaTChem_[celli] = tauC;
                dt = min(timeLeft, tauC);
                dt = max(dt, SMALL);
            }

            deltaTMin = min(tauC, deltaTMin);
            dc = c - c0;

            forAll(RRs_, i)
            {
                RRs_[i][celli] = dc[i]/(deltaT*delta);
            }

            forAll(RRg_, i)
            {
                RRg_[i][celli] = dc[nSolids_ + i]/(deltaT*delta);
            }

            // Update Ys0_
            dc = omega(c0, Ti, 0.0, true);
        }
    }

    // Don't allow the time-step to change more than a factor of 2
    deltaTMin = min(deltaTMin, 2*deltaT);

    return deltaTMin;
}


template<class CompType, class SolidThermo,class GasThermo>
Foam::tmp<Foam::volScalarField>
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::gasHs
(
    const volScalarField& T,
    const label index
) const
{

    tmp<volScalarField> tHs
    (
        new volScalarField
        (
            IOobject
            (
                "Hs_" + pyrolisisGases_[index],
                this->mesh_.time().timeName(),
                this->mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            this->mesh_,
            dimensionedScalar("zero", dimEnergy/dimMass, 0.0),
            zeroGradientFvPatchScalarField::typeName
        )
    );

    volScalarField& gasHs = tHs();

    const GasThermo& mixture = gasThermo_[index];

    forAll(gasHs.internalField(), cellI)
    {
        gasHs[cellI] = mixture.Hs(T[cellI]);
    }

    return tHs;
}


template<class CompType, class SolidThermo,class GasThermo>
Foam::scalar
Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::solve
(
    scalarField &c,
    const scalar T,
    const scalar p,
    const scalar t0,
    const scalar dt
) const
{
    notImplemented
    (
        "ODESolidChemistryModel::solve"
        "("
            "scalarField&, "
            "const scalar, "
            "const scalar, "
            "const scalar, "
            "const scalar"
        ")"
    );
    return (0);
}


template<class CompType, class SolidThermo,class GasThermo>
void Foam::ODESolidChemistryModel<CompType, SolidThermo, GasThermo>::
setCellReacting(const label cellI, const bool active)
{
    reactingCells_[cellI] = active;
}


// ************************************************************************* //
