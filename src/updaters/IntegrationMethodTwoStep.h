/*
Highly Optimized Object-oriented Many-particle Dynamics -- Blue Edition
(HOOMD-blue) Open Source Software License Copyright 2008, 2009 Ames Laboratory
Iowa State University and The Regents of the University of Michigan All rights
reserved.

HOOMD-blue may contain modifications ("Contributions") provided, and to which
copyright is held, by various Contributors who have granted The Regents of the
University of Michigan the right to modify and/or distribute such Contributions.

Redistribution and use of HOOMD-blue, in source and binary forms, with or
without modification, are permitted, provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions, and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
list of conditions, and the following disclaimer in the documentation and/or
other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of HOOMD-blue's
contributors may be used to endorse or promote products derived from this
software without specific prior written permission.

Disclaimer

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND/OR
ANY WARRANTIES THAT THIS SOFTWARE IS FREE OF INFRINGEMENT ARE DISCLAIMED.

IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// $Id$
// $URL$
// Maintainer: joaander

#include <boost/shared_ptr.hpp>

#include "SystemDefinition.h"
#include "ParticleGroup.h"
#include "Profiler.h"

#ifndef __INTEGRATION_METHOD_TWO_STEP_H__
#define __INTEGRATION_METHOD_TWO_STEP_H__

/*! \file IntegrationMethodTwoStep.h
    \brief Declares a base class for all two-step integration methods
*/

//! Integrates part of the system forward in two steps
/*! \b Overview
    A large class of integrators can be implemented in two steps:
    - Update position and velocity (w/ current accel)
    - Sum accelerations at the current position
    - Update position and velocity again (w/ newly calculated accel)
    
    It is also sometimes desierable to run part of the system with one integration method (i.e. NVE) and part with
    another (NPT). Or part of the system with an integrator and the other part none at all. To facilitate this, the
    IntegrationMethodTwoStep is being created. It is a generic class, of which sub classes (TwoStepNVT, TwoStepNVE, ...)
    will implement the specific two step integration method. A single integrator, IntegratorTwoStep, can contain
    several two step integration methods. It calls the first step on all of them, then calculates the forces, and then
    calls the second step on all methods. In this way, the entire system will be integrated forward correctly.
    
    This design is chosen so that a single integration method is only applied to a group of particles. To enforce this
    design constraint, the group is specified in the constructor to the base class method.
    
    However, some care needs to be put into thinking about the computation of the net force / accelerations. Prior to
    implementing IntegrationMethodTwoStep, Integrators on the CPU have the net force and acceleration summed in
    Integrator::computeAccelerations. While the GPU ones only compute the forces in that call, and sum the net force
    and acceleration within the 2nd step of the integrator. In an interaction with groups, this is not going to work
    out. If one integration method worked one way and another worked the other in the same IntegratorTwoStep, then
    the net force / acceleration would probably not be calculated properly. To avoid this problem, a net force and
    virial will summed as each force compute is called and will thus always be correct and up to date. Each individual
    integration method will then compute and fill out the particle's acceleration.
    
    One small note: each IntegrationTwoStep will have a deltaT. The value of this will be set by the integrator when
    Integrator::setDeltaT is called to ensure that all integration methods have the same delta t set.
    
    <b>Design requirements</b>
    Due to the nature of allowing multiple integration methods to run at once, some strict guidlines need to be laid
    down.
    -# All methods must use the same \a deltaT (this is enforced by having IntegratorTwoStep call setDeltaT on all of
       the methods inside it.
    -# integrateStepOne uses the particle acceleration currently set to advance particle positions forward one full
       step, and velocities are advanced forward a half step.
    -# integrateStepTwo assigns the current particle acceleration from the net force and updates the velocities
       forward for the second half step
    -# each integration method only applies these operations to the particles contained within its group (exceptions
       are allowed when box rescaling is needed)
    
    <b>Design items still left to do:</b>
    
    Interaction with logger: perhaps the integrator should forward log value queries on to the integration method? 
    each method could be given a user name so that they are logged in user-controlled columns. This provides a window
    into the interally computed state variables and conserved quantity logging per method.
    
    Computation of state variables: for now, any needed state variables will be computed internally as they have been
    in the past for maximum performance. With possible changes desired down the road to allow the user to control the
    DOF in the temperature calculation, this needs to be thought about. Maybe a full-blown Compute (w/ group setting)
    for temperature, pressure, kinetic energy, .... is needed even if it is going to mean some perf losses. Will need
    to evaluate what those losses would be before deciding.
    
    \ingroup updaters
*/
class IntegrationMethodTwoStep : boost::noncopyable
    {
    public:
        //! Constructs the integration method and associates it with the system
        IntegrationMethodTwoStep(boost::shared_ptr<SystemDefinition> sysdef, boost::shared_ptr<ParticleGroup> group);
        virtual ~IntegrationMethodTwoStep() {};
        
        //! Abstract method that performs the first step of the integration
        /*! \param timestep Current time step
        */
        virtual void integrateStepOne(unsigned int timestep) {}
        
        //! Abstract method that performs the second step of the integration
        /*! \param timestep Current time step
            \param net_force The net force on each particle in the system
            \param net_virial The net virial on each particle in the system
        */
        virtual void integrateStepTwo(unsigned int timestep,
                                      const GPUArray< Scalar4 >& net_force,
                                      const GPUArray< Scalar >& net_virial)
            {
            }
        
        //! Sets the profiler for the integration method to use
        void setProfiler(boost::shared_ptr<Profiler> prof);
        
        //! Change the timestep
        void setDeltaT(Scalar deltaT);
        
        //! Access the group
        boost::shared_ptr<ParticleGroup> getGroup() { return m_group; }
        
    protected:
        const boost::shared_ptr<SystemDefinition> m_sysdef; //!< The system definition this method is associated with
        const boost::shared_ptr<ParticleGroup> m_group;     //!< The group of particles this method works on
        const boost::shared_ptr<ParticleData> m_pdata;      //!< The particle data this method is associated with
        boost::shared_ptr<Profiler> m_prof;                 //!< The profiler this method is to use
        const ExecutionConfiguration& exec_conf;            //!< Cached reference to the execution configuration
        Scalar m_deltaT;                                    //!< The time step
    };

//! Exports the IntegrationMethodTwoStep class to python
void export_IntegrationMethodTwoStep();

#endif // #ifndef __INTEGRATION_METHOD_TWO_STEP_H__
