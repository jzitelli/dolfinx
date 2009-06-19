// Copyright (C) 2005-2008 Anders Logg.
// Licensed under the GNU LGPL Version 2.1.
//
// First added:  2005-01-05
// Last changed: 2008-06-11

#include <cmath>
#include <dolfin/common/constants.h>
#include <dolfin/parameter/parameters.h>
#include "TimeSlab.h"
#include "TimeSlabSolver.h"

using namespace dolfin;

//-----------------------------------------------------------------------------
TimeSlabSolver::TimeSlabSolver(TimeSlab& timeslab)
  : ode(timeslab.ode), method(*timeslab.method), tol(0.0), maxiter(0),
    monitor(ode.get("ODE monitor convergence")),
    num_timeslabs(0), num_global_iterations(0), num_local_iterations(0),
    xnorm(0.0)
{
  // Choose tolerance
  chooseTolerance();

  // Get maximum number of iterations
  maxiter = ode.get("ODE maximum iterations");
}
//-----------------------------------------------------------------------------
TimeSlabSolver::~TimeSlabSolver()
{
  if (num_timeslabs > 0)
  {
    const real n = static_cast<real>(num_timeslabs);
    const real global_average = static_cast<real>(num_global_iterations) / n;
    const real local_average = static_cast<real>(num_local_iterations) / 
      static_cast<real>(num_global_iterations);
    message("Average number of global iterations per step: %.3f",
	    to_double(global_average));
    message("Average number of local iterations per global iteration: %.3f",
	    to_double(local_average));
  }

  message("Total number of (macro) time steps: %d", num_timeslabs);
}
//-----------------------------------------------------------------------------
bool TimeSlabSolver::solve()
{
  for (uint attempt = 0; attempt < maxiter; attempt++)
  {
    // Try to solve system
    if ( solve(attempt) )
      return true;
    
    // Check if we should try again
    if ( !retry() )
      return false;
  }

  return false;
}
//-----------------------------------------------------------------------------
bool TimeSlabSolver::solve(uint attempt)
{
  start();

  real d0 = 0.0;
  real d1 = 0.0;
  for (uint iter = 0; iter < maxiter; iter++)
  {
    // Do one iteration
    real d2 = iteration(tol, iter, d0, d1);

    // Use relative increment
    d2 /= xnorm + real_epsilon();
    
    // For debugging convergence
    if (monitor)
      message("--- iter = %d: increment = %.3e", iter, to_double(d2));
    
    // Check convergenge
    if (d2 < tol)
    {
      end();
      num_timeslabs += 1;
      num_global_iterations += iter + 1;
      if (monitor)
	message("Time slab system of size %d converged in %d iterations.\n", size(), iter + 1);
      return true;
    }

    // Check divergence
    // FIXME: implement better check and make this a parameter
    // Is it ok to convert a gmp type to double and then check if it is normal?
    if ((iter > 0 && d2 > 1000.0 * d1) || !isnormal(d2))
    {
      warning("Time slab system seems to be diverging.");
      return false;
    }
    
    d0 = d1;
    d1 = d2;
  }

  warning("Time slab system did not converge.");
  return false;
}
//-----------------------------------------------------------------------------
bool TimeSlabSolver::retry()
{
  // By default, we don't know how to make a new attempt
  return false;
}
//-----------------------------------------------------------------------------
void TimeSlabSolver::start()
{
  // Do nothing
}
//-----------------------------------------------------------------------------
void TimeSlabSolver::end()
{
  // Do nothing
}
//-----------------------------------------------------------------------------
void TimeSlabSolver::chooseTolerance()
{
  const real TOL   = ode.get("ODE tolerance").get_real();
  const real alpha = ode.get("ODE discrete tolerance factor").get_real();

  tol =  ode.get("ODE discrete tolerance").get_real();

  if ( !ode.get("ODE fixed time step") )
  {
    tol = min(tol, alpha*TOL);
  }
  cout << "Using discrete tolerance tol = " << tol << "." << endl;
}
//-----------------------------------------------------------------------------
