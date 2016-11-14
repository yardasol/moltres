#include "ScalarAdvectionArtDiff.h"

template<>
InputParameters validParams<ScalarAdvectionArtDiff>()
{
  InputParameters params = validParams<Kernel>();
  params += ScalarTransportBase<Kernel>::validParams();
  params.addParam<Real>("scale", 1. ,"Amount to scale artificial diffusion.");

  // Coupled variables
  params.addCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity");
  params.addCoupledVar("w", "z-velocity");
  params.addParam<Real>("u_def", 0, "Allows user to specify constant value for u component of velocity.");
  params.addParam<Real>("v_def", 0, "Allows user to specify constant value for v component of velocity.");
  params.addParam<Real>("w_def", 0, "Allows user to specify constant value for w component of velocity.");
  return params;
}

ScalarAdvectionArtDiff::ScalarAdvectionArtDiff(const InputParameters & parameters) :
    ScalarTransportBase<Kernel>(parameters),

    _scale(getParam<Real>("scale")),
    // Coupled variables
    _u_vel(isCoupled("u") ? coupledValue("u") : _u_def),
    _v_vel(isCoupled("v") ? coupledValue("v") : _v_def),
    _w_vel(isCoupled("w") ? coupledValue("w") : _w_def),

    // Variable numberings
    _u_vel_var_number(coupled("u")),
    _v_vel_var_number(coupled("v")),
    _w_vel_var_number(coupled("w"))
{
  if (!(isCoupled("u")))
    _u_def.resize(_fe_problem.getMaxQps(), Real(getParam<Real>("u_def")));
  if (!(isCoupled("v")))
    _v_def.resize(_fe_problem.getMaxQps(), Real(getParam<Real>("v_def")));
  if (!(isCoupled("w")))
    _w_def.resize(_fe_problem.getMaxQps(), Real(getParam<Real>("w_def")));
}

Real
ScalarAdvectionArtDiff::computeQpResidual()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  Real  delta = U.norm() * _current_elem->hmax() / 2.;

  return -_grad_test[_i][_qp] * -delta * computeConcentrationGradient() * _scale;
}

Real
ScalarAdvectionArtDiff::computeQpJacobian()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  Real  delta = U.norm() * _current_elem->hmax() / 2.;

  return -_grad_test[_i][_qp] * -delta * computeConcentrationGradientDerivative() * _scale;
}

Real
ScalarAdvectionArtDiff::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _u_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real  d_delta_d_u_vel = _u_vel[_qp] * _phi[_j][_qp] / U.norm() * _current_elem->hmax() / 2.;
    return -_grad_test[_i][_qp] * -d_delta_d_u_vel * computeConcentrationGradient() * _scale;
  }

  else if (jvar == _v_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real  d_delta_d_v_vel = _v_vel[_qp] * _phi[_j][_qp] / U.norm() * _current_elem->hmax() / 2.;
    return -_grad_test[_i][_qp] * -d_delta_d_v_vel * computeConcentrationGradient() * _scale;
  }

  else if (jvar == _w_vel_var_number)
  {
    RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
    Real  d_delta_d_w_vel = _w_vel[_qp] * _phi[_j][_qp] / U.norm() * _current_elem->hmax() / 2.;
    return -_grad_test[_i][_qp] * -d_delta_d_w_vel * computeConcentrationGradient() * _scale;
  }
  else
    return 0.0;
}