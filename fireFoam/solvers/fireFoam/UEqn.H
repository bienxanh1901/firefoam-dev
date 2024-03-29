fvVectorMatrix UEqn
(
    fvm::ddt(rho, U)
  + fvm::div(phi, U)
  + turbulence->divDevRhoReff(U)
 == 
    parcels.SU()
);

if (oCorr == nOuterCorr-1)
{
    UEqn.relax(1);
}
else 
{
    UEqn.relax();
}

if (oCorr == nOuterCorr - 1)
{
    solve
    (
        UEqn
      ==
        fvc::reconstruct
        (
//            fvc::interpolate(rho)*(g & mesh.Sf())
//          - fvc::snGrad(p)*mesh.magSf()
            (
              - ghf*fvc::snGrad(rho)
              - fvc::snGrad(p_rgh)
            )*mesh.magSf()
        ),
        mesh.solver("UFinal")
    );
}
else
{
    solve
    (
        UEqn
      ==
        fvc::reconstruct
        (
//            fvc::interpolate(rho)*(g & mesh.Sf())
//          - fvc::snGrad(p)*mesh.magSf()
            (
              - ghf*fvc::snGrad(rho)
              - fvc::snGrad(p_rgh)
            )*mesh.magSf()
        )
    );
}
