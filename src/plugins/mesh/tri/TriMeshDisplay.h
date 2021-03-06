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

#pragma once


#include <plugins/mesh/Mesh.h>
#include <core/dataset/data/DisplayObject.h>
#include <core/dataset/data/VersionedDataObjectRef.h>
#include <core/rendering/MeshPrimitive.h>
#include <core/dataset/animation/controller/Controller.h>
#include <core/dataset/animation/AnimationSettings.h>

namespace Ovito { namespace Mesh {

/**
 * \brief A scene display object for triangle meshes.
 */
class OVITO_MESH_EXPORT TriMeshDisplay : public DisplayObject
{
	Q_OBJECT
	OVITO_CLASS(TriMeshDisplay)
	Q_CLASSINFO("DisplayName", "Triangle mesh");
	
public:

	/// \brief Constructor.
	Q_INVOKABLE TriMeshDisplay(DataSet* dataset);

	/// \brief Lets the display object render a data object.
	virtual void render(TimePoint time, DataObject* dataObject, const PipelineFlowState& flowState, SceneRenderer* renderer, ObjectNode* contextNode) override;

	/// \brief Computes the bounding box of the object.
	virtual Box3 boundingBox(TimePoint time, DataObject* dataObject, ObjectNode* contextNode, const PipelineFlowState& flowState, TimeInterval& validityInterval) override;

	/// Returns the transparency parameter.
	FloatType transparency() const { return transparencyController()->currentFloatValue(); }

	/// Sets the transparency parameter.
	void setTransparency(FloatType t) { transparencyController()->setCurrentFloatValue(t); }

private:

	/// Controls the display color of the mesh.
	DECLARE_MODIFIABLE_PROPERTY_FIELD_FLAGS(Color, color, setColor, PROPERTY_FIELD_MEMORIZE);

	/// Controls the transparency of the mesh.
	DECLARE_MODIFIABLE_REFERENCE_FIELD(Controller, transparencyController, setTransparencyController);

	/// The buffered geometry used to render the mesh.
	std::shared_ptr<MeshPrimitive> _buffer;

	/// This helper structure is used to detect any changes in the input data
	/// that require updating the geometry buffer.
	SceneObjectCacheHelper<
		VersionedDataObjectRef,		// Mesh object
		ColorA						// Display color
		> _geometryCacheHelper;
};

}	// End of namespace
}	// End of namespace
