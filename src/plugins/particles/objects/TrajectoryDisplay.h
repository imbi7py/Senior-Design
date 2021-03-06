///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (2015) Alexander Stukowski
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

#pragma once


#include <plugins/particles/Particles.h>
#include <core/dataset/data/DisplayObject.h>
#include <core/dataset/data/VersionedDataObjectRef.h>
#include <core/rendering/SceneRenderer.h>
#include "TrajectoryObject.h"

namespace Ovito { namespace Particles {

/**
 * \brief A display object for particle trajectories.
 */
class OVITO_PARTICLES_EXPORT TrajectoryDisplay : public DisplayObject
{
	Q_OBJECT
	OVITO_CLASS(TrajectoryDisplay)
	Q_CLASSINFO("DisplayName", "Trajectory lines");

public:

	/// \brief Constructor.
	Q_INVOKABLE TrajectoryDisplay(DataSet* dataset);

	/// \brief Renders the associated data object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the display bounding box of the data object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState, TimeInterval& validityInterval) override;

public:

    Q_PROPERTY(Ovito::ArrowPrimitive::ShadingMode shadingMode READ shadingMode WRITE setShadingMode);

protected:

	/// Controls the display width of trajectory lines.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(FloatType, lineWidth, setLineWidth, PROPERTY_FIELD_MEMORIZE);

	/// Controls the color of the trajectory lines.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(Color, lineColor, setLineColor, PROPERTY_FIELD_MEMORIZE);

	/// Controls the whether the trajectory lines are rendered only up to the current animation time.
	DECLARE_MODIFIABLE_PROPERTY_FIELD(bool, showUpToCurrentTime, setShowUpToCurrentTime);

	/// Controls the shading mode for lines.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(ArrowPrimitive::ShadingMode, shadingMode, setShadingMode, PROPERTY_FIELD_MEMORIZE);

	/// The buffered geometry used to render the trajectory lines.
	std::shared_ptr<ArrowPrimitive> _segmentBuffer;

	/// The buffered geometry used to render the trajectory line corners.
	std::shared_ptr<ParticlePrimitive> _cornerBuffer;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffers.
	SceneObjectCacheHelper<
		VersionedDataObjectRef,			// The trajectory data object + revision number
		FloatType,						// Line width
		Color,							// Line color,
		TimePoint						// End time
	> _geometryCacheHelper;

	/// The bounding box that includes all trajectories.
	Box3 _cachedBoundingBox;

	/// This helper structure is used to detect changes in the input data
	/// that require recomputing the bounding box.
	SceneObjectCacheHelper<
		VersionedDataObjectRef,		// The data object + revision number
		FloatType					// Line width
	> _boundingBoxCacheHelper;
};

}	// End of namespace
}	// End of namespace


