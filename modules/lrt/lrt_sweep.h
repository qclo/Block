/*                                                                           
Developed by Sandeep Sharma and Garnet K.-L. Chan, 2012                      
Copyright (c) 2012, Garnet K.-L. Chan                                        
                                                                             
This program is integrated in Molpro with the permission of 
Sandeep Sharma and Garnet K.-L. Chan
*/


#ifndef LRT_SPIN_SWEEP_HEADER
#define LRT_SPIN_SWEEP_HEADER
#include "spinblock.h"
#include "sweep_params.h"

namespace SpinAdapted {

namespace Sweep {

namespace LRT {

void BlockAndDecimate
(SweepParams &sweepParams, SpinBlock& system, SpinBlock& newSystem,
 const vector<double>& eigenvalues, vector<double>& rnorm, Matrix& h_subspace, Matrix& s_subspace, const Matrix& alpha,
 const bool &useSlater, const bool& dot_with_sys, int nroots, int mroots, int kroots, const bool& deflation_sweep);

double do_one(SweepParams &sweepParams, const bool &warmUp, const bool &forward, const bool &restart, const int &restartSize);

};

};

};

#endif
