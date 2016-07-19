// Copyright (c) 2009-2016 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

/*! \file MuellerPlatheFlowGPU.h

    \brief Declares a class to exchange velocities of
           different spatial region, to create a flow.
           GPU accelerated version.
*/


#ifdef ENABLE_CUDA
//Above this line shared constructs can be declared.
#ifndef NVCC
#include "hoomd/ParticleGroup.h"
#include "hoomd/Updater.h"
#include "hoomd/Variant.h"
#include "hoomd/Autotuner.h"
#include "MuellerPlatheFlow.h"

#include <cfloat>
#include <boost/shared_ptr.hpp>


#ifndef __MUELLER_PLATHE_FLOW_GPU_H__
#define __MUELLER_PLATHE_FLOW_GPU_H__

//! By exchanging velocities based on their spatial position a flow is created. GPU accelerated
/*! \ingroup computes
*/
class MuellerPlatheFlowGPU : public MuellerPlatheFlow
    {
    public:
        //! Constructs the compute
        //!
        //! \param direction Indicates the normal direction of the slabs.
        //! \param N_slabs Number of total slabs in the simulation box.
        //! \param min_slabs Index of slabs, where the min velocity is searched.
        //! \param max_slabs Index of slabs, where the max velocity is searched.
        //! \note N_slabs should be a multiple of the DomainDecomposition boxes in that direction.
        //! If it is not, the number is rescaled and the user is informed.
        MuellerPlatheFlowGPU(boost::shared_ptr<SystemDefinition> sysdef,
                             boost::shared_ptr<ParticleGroup> group,
                             boost::shared_ptr<Variant> flow_target,
                             const unsigned int slab_direction,
                             const unsigned int flow_direction,
                             const unsigned int N_slabs,
                             const unsigned int min_slab,
                             const unsigned int max_slab);

        //! Destructor
        virtual ~MuellerPlatheFlowGPU(void);

        //! Set autotuner parameters
        /*! \param enable Enable/disable autotuning
            \param period period (approximate) in time steps when returning occurs
        */
        virtual void setAutotunerParams(bool enable, unsigned int period)
            {
            MuellerPlatheFlow::setAutotunerParams(enable, period);
            m_tuner->setPeriod(period);
            m_tuner->setEnabled(enable);
            }


    protected:
        boost::scoped_ptr<Autotuner> m_tuner; //!< Autotuner for block size

        virtual void search_min_max_velocity(void);
        //virtual void update_min_max_velocity(void);
    };

//! Exports the MuellerPlatheFlow class to python
void export_MuellerPlatheFlowGPU();

#endif//NVCC
#endif//__MUELLER_PLATHE_FLOW_GPU_H__
#endif// ENABLE_CUDA
