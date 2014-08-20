/* -------------------------------------------------------------------------- *
*                       OpenSim:  BodyActuator.cpp                        *
* -------------------------------------------------------------------------- *
* The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
* See http://opensim.stanford.edu and the NOTICE file for more information.  *
* OpenSim is developed at Stanford University and supported by the US        *
* National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
* through the Warrior Web program.                                           *
*                                                                            *
* Copyright (c) 2014 Stanford University and the Authors                     *
* Author(s): Soha Pouya, Michael Sherman                                     *
*                                                                            *
* Licensed under the Apache License, Version 2.0 (the "License"); you may    *
* not use this file except in compliance with the License. You may obtain a  *
* copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
*                                                                            *
* Unless required by applicable law or agreed to in writing, software        *
* distributed under the License is distributed on an "AS IS" BASIS,          *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
* See the License for the specific language governing permissions and        *
* limitations under the License.                                             *
* -------------------------------------------------------------------------- */

//=============================================================================
// INCLUDES
//=============================================================================
#include <OpenSim/Common/XMLDocument.h>
#include <OpenSim/Simulation/Model/Model.h>
#include <OpenSim/Simulation/SimbodyEngine/Body.h>

#include "BodyActuator.h"

using namespace OpenSim;
using namespace std;
using SimTK::Vec3;


//=============================================================================
// CONSTRUCTORS
//=============================================================================
// Uses default (compiler-generated) destructor, copy constructor, copy 
// assignment operator.
//_____________________________________________________________________________
/**
* Default constructor.
*/
BodyActuator::BodyActuator()
{
	setAuthors("Soha Pouya, Michael Sherman");
	constructInfrastructure();
}
//_____________________________________________________________________________
/**
* Convenience constructor.
*/
BodyActuator::BodyActuator(const OpenSim::Body& body)
{
	setAuthors("Soha Pouya, Michael Sherman");
	constructInfrastructure();

	updConnector<Body>("body").set_connected_to_name(body.getName());
}

void BodyActuator::constructProperties()
{
}
//_____________________________________________________________________________
/**
* Construct Structural Connectors
*/
void BodyActuator::constructStructuralConnectors() {
	constructStructuralConnector<Body>("body");
}

void BodyActuator::setBodyName(const std::string& name)
{
	updConnector<Body>("body").set_connected_to_name(name);
}

const std::string& BodyActuator::getBodyName() const
{
	return getConnector<Body>("body").get_connected_to_name();
}

//=============================================================================
// GET AND SET
//=============================================================================
//_____________________________________________________________________________
/**
* Set the Body to which the BodyActuator is applied
*/
void BodyActuator::setBody(OpenSim::Body& body)
{
	updConnector<Body>("body").connect(body);
}

/**
* Get the Body to which the BodyActuator is applied
*/
const OpenSim::Body& BodyActuator::getBody() const
{
	return getConnector<Body>("body").getConnectee();
}

//==============================================================================
// APPLICATION
//==============================================================================
//_____________________________________________________________________________
/**
* Apply the actuator force/torque to Body.
*/
void BodyActuator::computeForce(const SimTK::State& s,
	SimTK::Vector_<SimTK::SpatialVec>& bodyForces,
	SimTK::Vector& generalizedForces) const
{
	const Body& body = getConnector<Body>("body").getConnectee();
	SimTK::MobilizedBodyIndex body_mbi = getModel().getBodySet().get(body.getName()).getIndex();
	const SimTK::MobilizedBody& body_mb = getModel().getMatterSubsystem().getMobilizedBody(body_mbi);

	Vec3 bodyOriginLocation = body_mb.getBodyOriginLocation(s);

	const SimTK::Vector bodyForceInGround = getControls(s);
	const Vec3 torqueVec_G(bodyForceInGround[0], bodyForceInGround[1], bodyForceInGround[2]);
	const Vec3 forceVec_G(bodyForceInGround[3], bodyForceInGround[4], bodyForceInGround[5]);

	applyTorque(s, body, torqueVec_G, bodyForces);
	applyForceToPoint(s, body, bodyOriginLocation, forceVec_G, bodyForces);

}

double BodyActuator::getPower(const SimTK::State& s) const
{
	const Body& body = getConnector<Body>("body").getConnectee();

	SimTK::MobilizedBodyIndex body_mbi = getModel().getBodySet().get(body.getName()).getIndex();
	const SimTK::MobilizedBody& body_mb = getModel().getMatterSubsystem().getMobilizedBody(body_mbi);
	SimTK::SpatialVec bodySpatialVelocities = body_mb.getBodyVelocity(s);

	SimTK::Vector bodyVelocityVec(6);
	bodyVelocityVec[0] = bodySpatialVelocities[0][0];
	bodyVelocityVec[1] = bodySpatialVelocities[0][1];
	bodyVelocityVec[2] = bodySpatialVelocities[0][2];
	bodyVelocityVec[3] = bodySpatialVelocities[1][0];
	bodyVelocityVec[4] = bodySpatialVelocities[1][1];
	bodyVelocityVec[5] = bodySpatialVelocities[1][2];

	const SimTK::Vector bodyForceVals = getControls(s);

	double power = 0.0;
	for (int i = 0; i < 6; i++){
		power += bodyForceVals[i] * bodyVelocityVec[i];
	}

	return power;
}
//_____________________________________________________________________________
/**
* Sets the actual Body reference 
*/
void BodyActuator::connectToModel(Model& model)
{
	Super::connectToModel(model);
}


