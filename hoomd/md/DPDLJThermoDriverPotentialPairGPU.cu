// Copyright (c) 2009-2019 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

/*! \file DPDLJThermoDriverPotentialPairGPU.cu
    \brief Defines the driver functions for computing all types of pair forces on the GPU
*/

#include "EvaluatorPairDPDLJThermo.h"
#include "AllDriverPotentialPairGPU.cuh"

cudaError_t gpu_compute_dpdljthermodpd_forces(const dpd_pair_args_t& args,
                                              const Scalar4 *d_params)
    {
    return gpu_compute_dpd_forces<EvaluatorPairDPDLJThermo>(args,
                                                            d_params);
    }


cudaError_t gpu_compute_dpdljthermo_forces(const pair_args_t& args,
                                           const Scalar4 *d_params)
    {
    return gpu_compute_pair_forces<EvaluatorPairDPDLJThermo>(args,
                                                             d_params);
    }

