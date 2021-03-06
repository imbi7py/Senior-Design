///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2013) Alexander Stukowski
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
#include <core/utilities/units/UnitsManager.h>
#include <core/utilities/concurrent/ParallelFor.h>
#include <core/rendering/SceneRenderer.h>
#include "VectorDisplay.h"
#include "ParticleDisplay.h"

namespace Ovito { namespace Particles {

IMPLEMENT_OVITO_CLASS(VectorDisplay);	
IMPLEMENT_OVITO_CLASS(VectorPickInfo);	
DEFINE_PROPERTY_FIELD(VectorDisplay, reverseArrowDirection);
DEFINE_PROPERTY_FIELD(VectorDisplay, arrowPosition);
DEFINE_PROPERTY_FIELD(VectorDisplay, arrowColor);
DEFINE_PROPERTY_FIELD(VectorDisplay, arrowWidth);
DEFINE_PROPERTY_FIELD(VectorDisplay, scalingFactor);
DEFINE_PROPERTY_FIELD(VectorDisplay, shadingMode);
DEFINE_PROPERTY_FIELD(VectorDisplay, renderingQuality);
SET_PROPERTY_FIELD_LABEL(VectorDisplay, arrowColor, "Arrow color");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, arrowWidth, "Arrow width");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, scalingFactor, "Scaling factor");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, reverseArrowDirection, "Reverse direction");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, arrowPosition, "Position");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, shadingMode, "Shading mode");
SET_PROPERTY_FIELD_LABEL(VectorDisplay, renderingQuality, "RenderingQuality");
SET_PROPERTY_FIELD_UNITS_AND_MINIMUM(VectorDisplay, arrowWidth, WorldParameterUnit, 0);
SET_PROPERTY_FIELD_UNITS_AND_MINIMUM(VectorDisplay, scalingFactor, FloatParameterUnit, 0);

/******************************************************************************
* Constructor.
******************************************************************************/
VectorDisplay::VectorDisplay(DataSet* dataset) : DisplayObject(dataset),
	_reverseArrowDirection(false), _arrowPosition(Base), _arrowColor(1, 1, 0), _arrowWidth(0.5), _scalingFactor(1),
	_shadingMode(ArrowPrimitive::FlatShading),
	_renderingQuality(ArrowPrimitive::LowQuality)
{
}

/******************************************************************************
* Computes the bounding box of the object.
******************************************************************************/
Box3 VectorDisplay::boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState, TimeInterval& validityInterval)
{
	ParticleProperty* vectorProperty = dynamic_object_cast<ParticleProperty>(dataObject);
	ParticleProperty* positionProperty = ParticleProperty::findInState(flowState, ParticleProperty::PositionProperty);
	if(vectorProperty && (vectorProperty->dataType() != PropertyStorage::Float || vectorProperty->componentCount() != 3))
		vectorProperty = nullptr;

	// Detect if the input data has changed since the last time we computed the bounding box.
	if(_boundingBoxCacheHelper.updateState(
			vectorProperty,
			positionProperty,
			scalingFactor(), arrowWidth()) || _cachedBoundingBox.isEmpty()) {
		// Recompute bounding box.
		_cachedBoundingBox = arrowBoundingBox(vectorProperty, positionProperty);
	}
	return _cachedBoundingBox;
}

/******************************************************************************
* Computes the bounding box of the arrows.
******************************************************************************/
Box3 VectorDisplay::arrowBoundingBox(ParticleProperty* vectorProperty, ParticleProperty* positionProperty)
{
	if(!positionProperty || !vectorProperty)
		return Box3();

	OVITO_ASSERT(positionProperty->type() == ParticleProperty::PositionProperty);
	OVITO_ASSERT(vectorProperty->dataType() == PropertyStorage::Float);
	OVITO_ASSERT(vectorProperty->componentCount() == 3);

	// Compute bounding box of particle positions (only those with non-zero vector).
	Box3 bbox;
	const Point3* p = positionProperty->constDataPoint3();
	const Point3* p_end = p + positionProperty->size();
	const Vector3* v = vectorProperty->constDataVector3();
	for(; p != p_end; ++p, ++v) {
		if((*v) != Vector3::Zero())
			bbox.addPoint(*p);
	}

	// Find largest vector magnitude.
	FloatType maxMagnitude = 0;
	const Vector3* v_end = vectorProperty->constDataVector3() + vectorProperty->size();
	for(v = vectorProperty->constDataVector3(); v != v_end; ++v) {
		FloatType m = v->squaredLength();
		if(m > maxMagnitude) maxMagnitude = m;
	}

	// Enlarge the bounding box by the largest vector magnitude + padding.
	return bbox.padBox((sqrt(maxMagnitude) * std::abs(scalingFactor())) + arrowWidth());
}

/******************************************************************************
* Lets the display object render the data object.
******************************************************************************/
void VectorDisplay::render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode)
{
	if(renderer->isBoundingBoxPass()) {
		TimeInterval validityInterval;
		renderer->addToLocalBoundingBox(boundingBox(time, dataObject, contextNode, flowState, validityInterval));
		return;
	}
	
	// Get input data.
	ParticleProperty* vectorProperty = dynamic_object_cast<ParticleProperty>(dataObject);
	ParticleProperty* positionProperty = ParticleProperty::findInState(flowState, ParticleProperty::PositionProperty);
	if(vectorProperty && (vectorProperty->dataType() != PropertyStorage::Float || vectorProperty->componentCount() != 3))
		vectorProperty = nullptr;
	ParticleProperty* vectorColorProperty = ParticleProperty::findInState(flowState, ParticleProperty::VectorColorProperty);

	// Make sure we don't exceed our internal limits.
	if(vectorProperty && vectorProperty->size() > (size_t)std::numeric_limits<int>::max()) {
		qWarning() << "WARNING: Cannot render more than" << std::numeric_limits<int>::max() << "vectors.";
		return;
	}
	
	// Do we have to re-create the geometry buffer from scratch?
	bool recreateBuffer = !_buffer || !_buffer->isValid(renderer);

	// Set shading mode and rendering quality.
	if(!recreateBuffer) {
		recreateBuffer |= !(_buffer->setShadingMode(shadingMode()));
		recreateBuffer |= !(_buffer->setRenderingQuality(renderingQuality()));
	}

	// Do we have to update contents of the geometry buffer?
	bool updateContents = _geometryCacheHelper.updateState(
			vectorProperty,
			positionProperty,
			scalingFactor(), arrowWidth(), arrowColor(), reverseArrowDirection(), arrowPosition(),
			vectorColorProperty)
			|| recreateBuffer;

	// Re-create the geometry buffer if necessary.
	if(recreateBuffer)
		_buffer = renderer->createArrowPrimitive(ArrowPrimitive::ArrowShape, shadingMode(), renderingQuality());

	// Update buffer contents.
	if(updateContents) {

		// Determine number of non-zero vectors.
		int vectorCount = 0;
		if(vectorProperty && positionProperty) {
			for(const Vector3& v : vectorProperty->constVector3Range()) {
				if(v != Vector3::Zero())
					vectorCount++;
			}
		}

		_buffer->startSetElements(vectorCount);
		if(vectorCount) {
			FloatType scalingFac = scalingFactor();
			if(reverseArrowDirection())
				scalingFac = -scalingFac;
			ColorA color(arrowColor());
			FloatType width = arrowWidth();
			ArrowPrimitive* buffer = _buffer.get();
			const Point3* pos = positionProperty->constDataPoint3();
			const Color* pcol = vectorColorProperty ? vectorColorProperty->constDataColor() : nullptr;
			int index = 0;
			for(const Vector3& vec : vectorProperty->constVector3Range()) {
				if(vec != Vector3::Zero()) {
					Vector3 v = vec * scalingFac;
					Point3 base = *pos;
					if(arrowPosition() == Head)
						base -= v;
					else if(arrowPosition() == Center)
						base -= v * FloatType(0.5);
					if(pcol)
						color = ColorA(*pcol);
					buffer->setElement(index++, base, v, color, width);
				}
				++pos;
				if(pcol) ++pcol;
			}
			OVITO_ASSERT(pos == positionProperty->constDataPoint3() + positionProperty->size());
			OVITO_ASSERT(!pcol || pcol == vectorColorProperty->constDataColor() + vectorColorProperty->size());
			OVITO_ASSERT(index == vectorCount);
		}
		_buffer->endSetElements();
	}

	if(renderer->isPicking()) {
		OORef<VectorPickInfo> pickInfo(new VectorPickInfo(this, flowState, vectorProperty));
		renderer->beginPickObject(contextNode, pickInfo);
	}
	_buffer->render(renderer);
	if(renderer->isPicking()) {
		renderer->endPickObject();
	}
}

/******************************************************************************
* Given an sub-object ID returned by the Viewport::pick() method, looks up the
* corresponding particle index.
******************************************************************************/
int VectorPickInfo::particleIndexFromSubObjectID(quint32 subobjID) const
{
	if(_vectorProperty) {
		int particleIndex = 0;
		for(const Vector3& v : _vectorProperty->constVector3Range()) {
			if(v != Vector3::Zero()) {
				if(subobjID == 0) return particleIndex;
				subobjID--;
			}
			particleIndex++;
		}
	}
	return -1;
}

/******************************************************************************
* Returns a human-readable string describing the picked object,
* which will be displayed in the status bar by OVITO.
******************************************************************************/
QString VectorPickInfo::infoString(ObjectNode* objectNode, quint32 subobjectId)
{
	int particleIndex = particleIndexFromSubObjectID(subobjectId);
	if(particleIndex < 0) return QString();
	return ParticlePickInfo::particleInfoString(pipelineState(), particleIndex);
}


}	// End of namespace
}	// End of namespace
