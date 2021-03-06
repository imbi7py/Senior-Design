///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2017) Alexander Stukowski
//
//  This file is part of OVITO (Open Visualization Tool).
//
//  OVITO is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  OVITO is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////

#include <plugins/particles/Particles.h>
#include <plugins/particles/modifier/ParticleInputHelper.h>
#include <plugins/particles/modifier/ParticleOutputHelper.h>
#include <core/dataset/animation/AnimationSettings.h>
#include <core/dataset/pipeline/ModifierApplication.h>
#include <plugins/stdobj/simcell/SimulationCellObject.h>
#include "InterpolateTrajectoryModifier.h"

namespace Ovito { namespace Particles { OVITO_BEGIN_INLINE_NAMESPACE(Modifiers) OVITO_BEGIN_INLINE_NAMESPACE(Properties)

IMPLEMENT_OVITO_CLASS(InterpolateTrajectoryModifier);
IMPLEMENT_OVITO_CLASS(InterpolateTrajectoryModifierApplication);
DEFINE_PROPERTY_FIELD(InterpolateTrajectoryModifier, useMinimumImageConvention);
SET_PROPERTY_FIELD_LABEL(InterpolateTrajectoryModifier, useMinimumImageConvention, "Use minimum image convention");

/******************************************************************************
* Constructs the modifier object.
******************************************************************************/
InterpolateTrajectoryModifier::InterpolateTrajectoryModifier(DataSet* dataset) : Modifier(dataset),
	_useMinimumImageConvention(true)
{
}

/******************************************************************************
* Asks the modifier whether it can be applied to the given input data.
******************************************************************************/
bool InterpolateTrajectoryModifier::OOMetaClass::isApplicableTo(const PipelineFlowState& input) const
{
	return input.findObject<ParticleProperty>() != nullptr;
}

/******************************************************************************
* Create a new modifier application that refers to this modifier instance.
******************************************************************************/
OORef<ModifierApplication> InterpolateTrajectoryModifier::createModifierApplication()
{
	OORef<ModifierApplication> modApp = new InterpolateTrajectoryModifierApplication(dataset());
	modApp->setModifier(this);
	return modApp;
}

/******************************************************************************
* Modifies the input data.
******************************************************************************/
Future<PipelineFlowState> InterpolateTrajectoryModifier::evaluate(TimePoint time, ModifierApplication* modApp, const PipelineFlowState& input)
{
	int nextFrame;
	SharedFuture<PipelineFlowState> nextStateFuture;

	// Determine the current frame, preferably from the attribute stored with the pipeline flow state.
	// If the source frame attribute is not present, fall back to inferring it from the current animation time.
	int currentFrame = input.sourceFrame();
	if(currentFrame < 0)
		currentFrame = modApp->animationTimeToSourceFrame(time);

	// If we are exactly on a source frame, there is no need to interpolate between two frames.
	if(modApp->sourceFrameToAnimationTime(currentFrame) == time) {
		nextFrame = currentFrame;
		nextStateFuture = input;
	}
	else {
		nextFrame = currentFrame + 1;

		// Obtain the subsequent input frame. 
		// Check if we already have the state in our cache available.
		if(InterpolateTrajectoryModifierApplication* myModApp = dynamic_object_cast<InterpolateTrajectoryModifierApplication>(modApp)) {
			if(myModApp->frameCache().sourceFrame() == nextFrame) {
				nextStateFuture = myModApp->frameCache();
			}
		}

		// Need to request next frame from the pipeline if not in cache.
		if(!nextStateFuture.isValid()) {
			nextStateFuture = modApp->evaluateInput(modApp->sourceFrameToAnimationTime(nextFrame));
		}
	}

	// Wait for the reference configuration to become available.
	return nextStateFuture.then(executor(), [this, time, modApp, input = input, nextFrame](const PipelineFlowState& nextState) mutable {
		if(InterpolateTrajectoryModifierApplication* myModApp = dynamic_object_cast<InterpolateTrajectoryModifierApplication>(modApp)) {
			
			// Make sure the obtained reference configuration is valid and ready to use.
			if(nextState.status().type() == PipelineStatus::Error)
				throwException(tr("Input state is not available: %1").arg(nextState.status().text()));
		
			if(nextState.sourceFrame() == nextFrame) {
				// Cache the next source frame in the ModifierApplication.
				myModApp->updateFrameCache(nextState);

				// Perform the actual interpolation.
				return evaluatePreliminary(time, modApp, input);
			}

			myModApp->invalidateFrameCache();
		}
		input.intersectStateValidity(time);
		return std::move(input);
	});
}

/******************************************************************************
* Modifies the input data in an immediate, preliminary way.
******************************************************************************/
PipelineFlowState InterpolateTrajectoryModifier::evaluatePreliminary(TimePoint time, ModifierApplication* modApp, const PipelineFlowState& input)
{
	PipelineFlowState output = input;
	ParticleInputHelper pih(dataset(), input);
	ParticleOutputHelper poh(dataset(), output);
	
	// Determine the current frame, preferably from the attribute stored with the pipeline flow state.
	// If the source frame attribute is not present, fall back to inferring it from the current animation time.
	int currentFrame = input.sourceFrame();
	if(currentFrame < 0)
		currentFrame = modApp->animationTimeToSourceFrame(time);
	
	// If we are exactly on a source frame, there is no need to interpolate between two frames.
	if(modApp->sourceFrameToAnimationTime(currentFrame) == time) {
		output.intersectStateValidity(time);
		return output;
	}

	// Retrieve the state of the second frame stored in the ModifierApplication.
	int nextFrame = currentFrame + 1;
	InterpolateTrajectoryModifierApplication* myModApp = dynamic_object_cast<InterpolateTrajectoryModifierApplication>(modApp);
	if(!myModApp || myModApp->frameCache().sourceFrame() != nextFrame)
		throwException(tr("No frame state stored."));

	const PipelineFlowState& secondState = myModApp->frameCache();
	TimePoint time1 = modApp->sourceFrameToAnimationTime(currentFrame);
	TimePoint time2 = modApp->sourceFrameToAnimationTime(nextFrame);
	FloatType t = (FloatType)(time - time1) / (time2 - time1);
	if(t < 0) t = 0;
	else if(t > 1) t = 1;
	
	SimulationCellObject* cell1 = input.findObject<SimulationCellObject>();
	SimulationCellObject* cell2 = secondState.findObject<SimulationCellObject>();

	// Interpolate particle positions.
	ParticleProperty* posProperty1 = pih.expectStandardProperty<ParticleProperty>(ParticleProperty::PositionProperty);
	ParticleProperty* posProperty2 = ParticleProperty::findInState(secondState, ParticleProperty::PositionProperty);
	if(!posProperty2 || posProperty1->size() != posProperty2->size())
		throwException(tr("Cannot interpolate between consecutive frames, because they contain different numbers of particles"));

	ParticleProperty* idProperty1 = ParticleProperty::findInState(input, ParticleProperty::IdentifierProperty);
	ParticleProperty* idProperty2 = ParticleProperty::findInState(secondState, ParticleProperty::IdentifierProperty);
	ParticleProperty* outputPositions = poh.outputStandardProperty<ParticleProperty>(ParticleProperty::PositionProperty, true);
	if(idProperty1 && idProperty2 && idProperty1->size() == idProperty2->size() &&  
			!std::equal(idProperty1->constDataInt(), idProperty1->constDataInt() + idProperty1->size(), idProperty2->constDataInt())) {

		// Build ID-to-index map.
		std::unordered_map<int,int> idmap;
		int index = 0;
		for(int id : idProperty2->constIntRange()) {
			if(!idmap.insert(std::make_pair(id,index)).second)
				throwException(tr("Detected duplicate particle ID %1.").arg(id));
			index++;
		}

		if(useMinimumImageConvention() && cell1 != nullptr) {
			SimulationCell cell = cell1->data();
			const int* id = idProperty1->constDataInt();
			for(Point3& p1 : outputPositions->point3Range()) {
				auto mapEntry = idmap.find(*id);
				if(mapEntry == idmap.end())
					throwException(tr("Cannot interpolate between consecutive frames, because the identity of particles changes."));
				Vector3 delta = cell.wrapVector(posProperty2->getPoint3(mapEntry->second) - p1);
				p1 += delta * t;
				++id;
			}
		}
		else {
			const int* id = idProperty1->constDataInt();
			for(Point3& p1 : outputPositions->point3Range()) {
				auto mapEntry = idmap.find(*id);
				if(mapEntry == idmap.end())
					throwException(tr("Cannot interpolate between consecutive frames, because the identity of particles changes."));
				p1 += (posProperty2->getPoint3(mapEntry->second) - p1) * t;
				++id;
			}
		}		
	}
	else {
		const Point3* p2 = posProperty2->constDataPoint3();
		if(useMinimumImageConvention() && cell1 != nullptr) {
			SimulationCell cell = cell1->data();
			for(Point3& p1 : outputPositions->point3Range()) {
				Vector3 delta = cell.wrapVector((*p2++) - p1);
				p1 += delta * t;
			}
		}
		else {
			for(Point3& p1 : outputPositions->point3Range()) {
				p1 += ((*p2++) - p1) * t;
			}
		}
	}

	// Interpolate simulation cell vectors.
	if(cell1 && cell2) {
		SimulationCellObject* outputCell = poh.outputObject<SimulationCellObject>();
		const AffineTransformation cellMat1 = cell1->cellMatrix();
		const AffineTransformation delta = cell2->cellMatrix() - cellMat1;
		outputCell->setCellMatrix(cellMat1 + delta * t);
	}
	
	output.intersectStateValidity(time);
	return output;
}

/******************************************************************************
* Is called when a RefTarget referenced by this object has generated an event.
******************************************************************************/
bool InterpolateTrajectoryModifierApplication::referenceEvent(RefTarget* source, ReferenceEvent* event)
{
	if(event->type() == ReferenceEvent::TargetChanged) {
		invalidateFrameCache();
	}
	return ModifierApplication::referenceEvent(source, event);
}

OVITO_END_INLINE_NAMESPACE
OVITO_END_INLINE_NAMESPACE
}	// End of namespace
}	// End of namespace
